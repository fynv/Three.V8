#include "MMIOContext.h"

extern "C" {
#include <libavformat/avformat.h>
}

#include "utils/Semaphore.h"

MMIOContext::MMIOContext(size_t buf_size,
	int (*read)(void* opaque, uint8_t* buf, int buf_size),
	int64_t(*seek)(void* opaque, int64_t offset, int whence))
	: buffer_size_(buf_size)	
	, ctx_(avio_alloc_context((unsigned char*)av_malloc(buffer_size_), buffer_size_, 0, this, read, NULL, seek))
{

}

MMIOContext::~MMIOContext()
{
	av_free(ctx_->buffer);
	av_free(ctx_);
}

MMFileContext::MMFileContext(const char* filename, size_t buf_size)
	: MMIOContext(buf_size, read, seek)
	, m_fp(fopen(filename, "rb"))
{
	
}

MMFileContext::~MMFileContext()
{
	fclose(m_fp);
}


int MMFileContext::read(void* opaque, unsigned char* buf, int buf_size)
{
	MMFileContext* self = (MMFileContext*)opaque;
	return fread(buf, 1, buf_size, self->m_fp);
}

int64_t MMFileContext::seek(void* opaque, int64_t offset, int whence)
{
	MMFileContext* self = (MMFileContext*)opaque;
	if (whence == AVSEEK_SIZE)
	{
		int64_t cur = ftell(self->m_fp);
		fseek(self->m_fp, 0, SEEK_END);
		int64_t size = ftell(self->m_fp);
		fseek(self->m_fp, cur, SEEK_SET);
		return size;
	}
	return fseek(self->m_fp, offset, whence);
}

#define BLOCK_SIZE 65536
#define MIN_CACHED_BLOCKS 16

MMFileStreamContext::MMFileStreamContext(const char* filename, size_t buf_size) 
	: MMIOContext(buf_size, read, seek)	
	, m_filename(filename)
	, m_sem_req(new Semaphore)
	, m_sem_ready(new Semaphore)
{
	m_reading = true;
	m_thread_cache = (std::unique_ptr<std::thread>)(new std::thread(thread_cache, this));
	m_sem_ready->wait();
}

MMFileStreamContext::~MMFileStreamContext()
{
	m_reading = false;
	m_sem_req->notify();
	m_thread_cache->join();
}

void MMFileStreamContext::thread_cache(MMFileStreamContext* self)
{
	self->m_fp = fopen(self->m_filename.c_str(), "rb");
	fseek(self->m_fp, 0, SEEK_END);
	self->m_file_size = ftell(self->m_fp);
	fseek(self->m_fp, 0, SEEK_SET);
	self->m_sem_ready->notify();

	self->m_cache.resize(BLOCK_SIZE * MIN_CACHED_BLOCKS);

	bool pending = false;
	size_t pos_start = 0;
	size_t size = 0;

	while (self->m_reading)
	{
		if (self->m_sem_req->get_count() > 0)
		{
			self->m_sem_req->wait();			

			int block_start = self->m_cur_pos / BLOCK_SIZE;
			int block_end = (self->m_cur_pos + self->m_req_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

			pos_start = block_start * BLOCK_SIZE;
			size = (block_end - block_start) * BLOCK_SIZE;

			if (size > self->m_cache.size())
			{
				self->m_cache.resize(size);
				self->m_cache_pos_file = pos_start;
				self->m_cache_pos = 0;
				self->m_cached_size = 0;
			}
			else if (pos_start < self->m_cache_pos_file)
			{
				self->m_cache_pos_file = pos_start;
				self->m_cache_pos = 0;
				self->m_cached_size = 0;
			}
			else
			{
				size_t delta = pos_start - self->m_cache_pos_file;
				self->m_cache_pos_file += delta;
				self->m_cache_pos = (self->m_cache_pos + delta) % self->m_cache.size();
				if (delta < self->m_cached_size)
				{
					self->m_cached_size -= delta;
				}
				else
				{
					self->m_cached_size = 0;
				}
			}

			pending = true;			
		}

		if (pending && self->m_cached_size >= size)
		{
			self->m_sem_ready->notify();
			pending = false;
		}

		if (self->m_cache_pos_file + self->m_cached_size < self->m_file_size 
			&& self->m_cached_size < self->m_cache.size())
		{
			size_t read_pos_file = self->m_cache_pos_file + self->m_cached_size;
			size_t read_size = self->m_file_size - read_pos_file;
			if (read_size > BLOCK_SIZE) read_size = BLOCK_SIZE;
			size_t read_pos = (self->m_cache_pos + self->m_cached_size) % self->m_cache.size();
			fseek(self->m_fp, read_pos_file, SEEK_SET);
			fread(self->m_cache.data() + read_pos, 1, read_size, self->m_fp);
			self->m_cached_size += BLOCK_SIZE;
		}
		else
		{
			self->m_sem_req->wait();
			self->m_sem_req->notify();
		}

	}

	fclose(self->m_fp);
}

int MMFileStreamContext::read(void* opaque, unsigned char* buf, int buf_size)
{
	MMFileStreamContext* self = (MMFileStreamContext*)opaque;
	self->m_req_size = buf_size;
	self->m_sem_req->notify();
	self->m_sem_ready->wait();
	size_t read_pos = (self->m_cur_pos - self->m_cache_pos_file + self->m_cache_pos) % self->m_cache.size();
	if (read_pos + buf_size <= self->m_cache.size())
	{
		memcpy(buf, self->m_cache.data() + read_pos, buf_size);
	}
	else
	{
		size_t tail = self->m_cache.size() - read_pos;
		memcpy(buf, self->m_cache.data() + read_pos, tail);
		memcpy(buf + tail, self->m_cache.data(), buf_size - tail);

	}
	self->m_cur_pos += buf_size;
	return buf_size;
}

int64_t MMFileStreamContext::seek(void* opaque, int64_t offset, int whence)
{
	MMFileStreamContext* self = (MMFileStreamContext*)opaque;
	if (whence == AVSEEK_SIZE)
	{
		return self->m_file_size;
	}

	if (whence == SEEK_SET)
	{
		self->m_cur_pos = offset;
	}
	if (whence == SEEK_CUR)
	{
		self->m_cur_pos = self->m_cur_pos + offset;
	}
	else if (whence == SEEK_END)
	{
		self->m_cur_pos = self->m_file_size + offset;
	}
	return self->m_cur_pos;
}

#include "network/HttpClient.h"

MMHttpStreamContext::MMHttpStreamContext(HttpClient* http, const char* url, size_t buf_size)
	: MMIOContext(buf_size, read, seek)
	, m_http(http)
	, m_url(url)
	, m_sem_req(new Semaphore)
	, m_sem_ready(new Semaphore)
{
	m_reading = true;
	m_thread_cache = (std::unique_ptr<std::thread>)(new std::thread(thread_cache, this));
	m_sem_ready->wait();
}

MMHttpStreamContext::~MMHttpStreamContext()
{
	m_reading = false;
	m_sem_req->notify();
	m_thread_cache->join();
}


void MMHttpStreamContext::thread_cache(MMHttpStreamContext* self)
{
	std::unordered_map<std::string, std::string> headers;
	self->m_http->GetHeaders(self->m_url.c_str(), headers);
	self->m_file_size = atoi(headers["Content-Length"].c_str());
	self->m_sem_ready->notify();

	self->m_cache.resize(BLOCK_SIZE * MIN_CACHED_BLOCKS);

	bool pending = false;
	size_t pos_start = 0;
	size_t size = 0;

	while (self->m_reading)
	{
		if (self->m_sem_req->get_count() > 0)
		{
			self->m_sem_req->wait();

			int block_start = self->m_cur_pos / BLOCK_SIZE;
			int block_end = (self->m_cur_pos + self->m_req_size + BLOCK_SIZE - 1) / BLOCK_SIZE;

			pos_start = block_start * BLOCK_SIZE;
			size = (block_end - block_start) * BLOCK_SIZE;

			if (size > self->m_cache.size())
			{
				self->m_cache.resize(size);
				self->m_cache_pos_file = pos_start;
				self->m_cache_pos = 0;
				self->m_cached_size = 0;
			}
			else if (pos_start < self->m_cache_pos_file)
			{
				self->m_cache_pos_file = pos_start;
				self->m_cache_pos = 0;
				self->m_cached_size = 0;
			}
			else
			{
				size_t delta = pos_start - self->m_cache_pos_file;
				self->m_cache_pos_file += delta;
				self->m_cache_pos = (self->m_cache_pos + delta) % self->m_cache.size();
				if (delta < self->m_cached_size)
				{
					self->m_cached_size -= delta;
				}
				else
				{
					self->m_cached_size = 0;
				}
			}

			pending = true;
		}

		if (pending && self->m_cached_size >= size)
		{
			self->m_sem_ready->notify();
			pending = false;
		}

		if (self->m_cache_pos_file + self->m_cached_size < self->m_file_size
			&& self->m_cached_size < self->m_cache.size())
		{
			size_t read_pos_file = self->m_cache_pos_file + self->m_cached_size;
			size_t read_size = self->m_file_size - read_pos_file;
			if (read_size > BLOCK_SIZE) read_size = BLOCK_SIZE;
			size_t read_pos = (self->m_cache_pos + self->m_cached_size) % self->m_cache.size();
			std::vector<unsigned char> buffer;
			self->m_http->GetRange(self->m_url.c_str(), read_pos_file, read_size, buffer);
			memcpy(self->m_cache.data() + read_pos, buffer.data(), read_size);
			self->m_cached_size += BLOCK_SIZE;
		}
		else
		{
			self->m_sem_req->wait();
			self->m_sem_req->notify();
		}

	}
}


int MMHttpStreamContext::read(void* opaque, unsigned char* buf, int buf_size)
{
	MMHttpStreamContext* self = (MMHttpStreamContext*)opaque;
	self->m_req_size = buf_size;
	self->m_sem_req->notify();
	self->m_sem_ready->wait();
	size_t read_pos = (self->m_cur_pos - self->m_cache_pos_file + self->m_cache_pos) % self->m_cache.size();
	if (read_pos + buf_size <= self->m_cache.size())
	{
		memcpy(buf, self->m_cache.data() + read_pos, buf_size);
	}
	else
	{
		size_t tail = self->m_cache.size() - read_pos;
		memcpy(buf, self->m_cache.data() + read_pos, tail);
		memcpy(buf + tail, self->m_cache.data(), buf_size - tail);

	}
	self->m_cur_pos += buf_size;
	return buf_size;
}

int64_t MMHttpStreamContext::seek(void* opaque, int64_t offset, int whence)
{
	MMHttpStreamContext* self = (MMHttpStreamContext*)opaque;
	if (whence == AVSEEK_SIZE)
	{
		return self->m_file_size;
	}

	if (whence == SEEK_SET)
	{
		self->m_cur_pos = offset;
	}
	if (whence == SEEK_CUR)
	{
		self->m_cur_pos = self->m_cur_pos + offset;
	}
	else if (whence == SEEK_END)
	{
		self->m_cur_pos = self->m_file_size + offset;
	}
	return self->m_cur_pos;
}