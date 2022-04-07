#pragma once

#include <cstdint>

class GLShader
{
public:
	unsigned m_type = 0;
	unsigned m_id = -1;
	GLShader(unsigned type, const char* code);
	~GLShader();
private:
	GLShader(const GLShader&);
};


class GLProgram
{
public:
	unsigned m_id = -1;
	GLProgram(const GLShader& vertexShader, const GLShader& fragmentShader);
	GLProgram(const GLShader& vertexShader, const GLShader& geometryShader, const GLShader& fragmentShader);
	GLProgram(const GLShader& computeShader);
	~GLProgram();
private:
	GLProgram(const GLProgram&);
};

class GLTexture2D
{
public:
	unsigned tex_id;
	GLTexture2D();
	~GLTexture2D();

	void load_memory_rgb(int width, int height, uint8_t* data, bool is_srgb);
	void load_file(const char* filename, bool is_srgb);
private:
	GLTexture2D(const GLTexture2D&);
};


class GLBuffer
{
public:
	unsigned m_id = -1;
	unsigned m_target = 0x8892;
	size_t m_size = 0;
	GLBuffer(size_t size, unsigned target = 0x8892 /*GL_ARRAY_BUFFER*/);
	~GLBuffer();
	void upload(const void* data);
	const GLBuffer& operator = (const GLBuffer& in);
private:
	GLBuffer(const GLBuffer&);
};

class GLDynBuffer
{
public:
	unsigned m_id = -1;
	unsigned m_target = 0x8892;
	size_t m_size = 0;
	GLDynBuffer(size_t size, unsigned target = 0x8892 /*GL_ARRAY_BUFFER*/);
	~GLDynBuffer();
	void upload(const void* data);
	const GLDynBuffer& operator = (const GLDynBuffer& in);
private:
	GLDynBuffer(const GLDynBuffer&);
};

