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

	void load_memory_rgb(int width, int height, const uint8_t* data, bool is_srgb);
	void load_memory_rgba(int width, int height, const uint8_t* data, bool is_srgb);
	void load_memory_bgr(int width, int height, const uint8_t* data, bool is_srgb);
	void load_memory_bgra(int width, int height, const uint8_t* data, bool is_srgb);
	void load_file(const char* filename, bool is_srgb);

	void unload();

private:
	GLTexture2D(const GLTexture2D&);
};

class GLTexture3D
{
public:
	unsigned tex_id;
	GLTexture3D();
	~GLTexture3D();

	void load_memory(int width, int height, int depth, const uint8_t* data, int bytes_per_pixel = 1);
	void unload();

private:
	GLTexture3D(const GLTexture3D&);
};

class GLCubemap
{
public:
	unsigned tex_id;
	GLCubemap();
	~GLCubemap();

	void load_memory_rgba(int width, int height, const uint8_t* data_xp, const uint8_t* data_xn, const uint8_t* data_yp, const uint8_t* data_yn, const uint8_t* data_zp, const uint8_t* data_zn);
	void load_memory_rgbe(int width, int height, const float* data_xp, const float* data_xn, const float* data_yp, const float* data_yn, const float* data_zp, const float* data_zn);
	void load_files(const char* fn_xp, const char* fn_xn, const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn);
	void load_rgbe_files(const char* fn_xp, const char* fn_xn, const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn);
	void unload();

private:
	GLCubemap(const GLCubemap&);
};

class ReflectionMap
{
public:
	unsigned tex_id;
	unsigned tex_id_dis;
	ReflectionMap(bool has_distance = false);
	~ReflectionMap();

	bool allocated = false;
	void allocate();

};

class GLBuffer
{
public:
	unsigned m_id = -1;
	unsigned m_target = 0x8892;
	size_t m_size = 0;
	GLBuffer(size_t size, unsigned target = 0x8892 /*GL_ARRAY_BUFFER*/);
	virtual ~GLBuffer();
	void upload(const void* data);
	const GLBuffer& operator = (const GLBuffer& in);
private:
	GLBuffer(const GLBuffer&);
};


class TextureBuffer : public GLBuffer
{
public:
	unsigned tex_id;
	TextureBuffer(size_t size, unsigned internalFormat);
	~TextureBuffer();

};
