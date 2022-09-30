#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <thread>

struct AVIOContext;
class MMIOContext
{
public:
	MMIOContext(size_t buf_size, 
		int (*read)(void* opaque, uint8_t* buf, int buf_size),
		int64_t(*seek)(void* opaque, int64_t offset, int whence));

	virtual ~MMIOContext();

	AVIOContext* get_avio() { return ctx_; }

protected:
	int buffer_size_;
	AVIOContext* ctx_ = nullptr;
};

class MMFileContext : public MMIOContext
{
public:
	MMFileContext(const char* filename, size_t buf_size = 4096);
	virtual ~MMFileContext();


private:
	FILE* m_fp;
	static int read(void* opaque, unsigned char* buf, int buf_size);
	static int64_t seek(void* opaque, int64_t offset, int whence);

};

class Semaphore;
class MMFileStreamContext : public MMIOContext
{
public:
	MMFileStreamContext(const char* filename, size_t buf_size = 4096);
	virtual ~MMFileStreamContext();


private:
	// file state
	std::string m_filename;
	FILE* m_fp = nullptr;
	size_t m_file_size = 0;

	// read state
	size_t m_cur_pos = 0;
	size_t m_req_size = 0;

	// cache state
	std::vector<uint8_t> m_cache;
	size_t m_cache_pos = 0;
	size_t m_cache_pos_file = 0;
	size_t m_cached_size = 0;

	// threading	
	bool m_reading = false;
	std::unique_ptr<Semaphore> m_sem_req;
	std::unique_ptr<Semaphore> m_sem_ready;

	static void thread_cache(MMFileStreamContext* self);
	std::unique_ptr<std::thread> m_thread_cache;

	static int read(void* opaque, unsigned char* buf, int buf_size);
	static int64_t seek(void* opaque, int64_t offset, int whence);
};


class HttpClient;
class MMHttpStreamContext : public MMIOContext
{
public:
	MMHttpStreamContext(HttpClient* http, const char* url, size_t buf_size = 4096);
	virtual ~MMHttpStreamContext();

private:	
	HttpClient* m_http;
	std::string m_url;
	size_t m_file_size = 0;

	// read state
	size_t m_cur_pos = 0;
	size_t m_req_size = 0;

	// cache state
	std::vector<uint8_t> m_cache;
	size_t m_cache_pos = 0;
	size_t m_cache_pos_file = 0;
	size_t m_cached_size = 0;

	// threading	
	bool m_reading = false;
	std::unique_ptr<Semaphore> m_sem_req;
	std::unique_ptr<Semaphore> m_sem_ready;

	static void thread_cache(MMHttpStreamContext* self);
	std::unique_ptr<std::thread> m_thread_cache;

	static int read(void* opaque, unsigned char* buf, int buf_size);
	static int64_t seek(void* opaque, int64_t offset, int whence);
};
