#include "MMIOContext.h"

extern "C" {
#include <libavformat/avformat.h>
}

MMFILEContext::MMFILEContext(const char* filename, size_t buf_size)
	: m_fp(fopen(filename, "rb"))
	, buffer_size_(buf_size)
	, buffer_((unsigned char*)av_malloc(buffer_size_))
{
	ctx_ = ::avio_alloc_context(buffer_, buffer_size_, 0, this,	read, NULL, seek);
}

MMFILEContext::~MMFILEContext()
{
	fclose(m_fp);
}


int MMFILEContext::read(void* opaque, unsigned char* buf, int buf_size)
{
	MMFILEContext* self = (MMFILEContext*)opaque;
	return fread(buf, 1, buf_size, self->m_fp);
}

int64_t MMFILEContext::seek(void* opaque, int64_t offset, int whence)
{
	MMFILEContext* self = (MMFILEContext*)opaque;
	if (whence == AVSEEK_SIZE)
	{
		int64_t cur = ftell(self->m_fp);
		fseek(self->m_fp, 0, SEEK_END);
		int64_t end = ftell(self->m_fp);
		fseek(self->m_fp, cur, SEEK_SET);
		return end - cur;
	}
	return fseek(self->m_fp, offset, whence);
}