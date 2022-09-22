#pragma once

#include <cstdint>
#include <cstdio>

struct AVIOContext;
class MMIOContext
{
public:
	MMIOContext() {}
	virtual ~MMIOContext() {}

	AVIOContext* get_avio() { return ctx_; }

protected:
	AVIOContext* ctx_ = nullptr;
};

class MMFILEContext : public MMIOContext
{
public:
	MMFILEContext(const char* filename, size_t buf_size = 4096);
	virtual ~MMFILEContext();


private:
	FILE* m_fp;
	int buffer_size_;
	unsigned char* buffer_;

	static int read(void* opaque, unsigned char* buf, int buf_size);
	static int64_t seek(void* opaque, int64_t offset, int whence);

};

