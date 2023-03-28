#include <GL/glew.h>
#include "utils/Utils.h"
#include "GLUtils.h"
#include <cstdio>
#include <vector>
#include <string>

#include "utils/Image.h"
#include "utils/HDRImage.h"
#include "loaders/ImageLoader.h"
#include "loaders/HDRImageLoader.h"

GLShader::GLShader(unsigned type, const char* code)
{
	m_type = type;
	m_id = glCreateShader(type);
	glShaderSource(m_id, 1, &code, nullptr);
	glCompileShader(m_id);

	GLint compileResult;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &compileResult);
	if (compileResult == 0)
	{
		GLint infoLogLength;
		glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<GLchar> infoLog(infoLogLength);
		glGetShaderInfoLog(m_id, (GLsizei)infoLog.size(), NULL, infoLog.data());

		printf("Shader compilation failed: %s", std::string(infoLog.begin(), infoLog.end()).c_str());
	}
}

GLShader::~GLShader()
{
	if (m_id != -1)
		glDeleteShader(m_id);
}


GLProgram::GLProgram(const GLShader& vertexShader, const GLShader& fragmentShader)
{
	m_id = glCreateProgram();
	glAttachShader(m_id, vertexShader.m_id);
	glAttachShader(m_id, fragmentShader.m_id);
	glLinkProgram(m_id);

	GLint linkResult;
	glGetProgramiv(m_id, GL_LINK_STATUS, &linkResult);
	if (linkResult == 0)
	{
		GLint infoLogLength;
		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(m_id, (GLsizei)infoLog.size(), NULL, infoLog.data());

		printf("Shader link failed: %s", std::string(infoLog.begin(), infoLog.end()).c_str());
	}
	glDetachShader(m_id, vertexShader.m_id);
	glDetachShader(m_id, fragmentShader.m_id);
}

GLProgram::GLProgram(const GLShader& vertexShader, const GLShader& geometryShader, const GLShader& fragmentShader)
{
	m_id = glCreateProgram();
	glAttachShader(m_id, vertexShader.m_id);
	glAttachShader(m_id, geometryShader.m_id);
	glAttachShader(m_id, fragmentShader.m_id);
	glLinkProgram(m_id);

	GLint linkResult;
	glGetProgramiv(m_id, GL_LINK_STATUS, &linkResult);
	if (linkResult == 0)
	{
		GLint infoLogLength;
		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(m_id, (GLsizei)infoLog.size(), NULL, infoLog.data());

		printf("Shader link failed: %s", std::string(infoLog.begin(), infoLog.end()).c_str());
	}
	glDetachShader(m_id, vertexShader.m_id);
	glDetachShader(m_id, geometryShader.m_id);
	glDetachShader(m_id, fragmentShader.m_id);
}

GLProgram::GLProgram(const GLShader& computeShader)
{
	m_id = glCreateProgram();
	glAttachShader(m_id, computeShader.m_id);
	glLinkProgram(m_id);

	GLint linkResult;
	glGetProgramiv(m_id, GL_LINK_STATUS, &linkResult);
	if (linkResult == 0)
	{
		GLint infoLogLength;
		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::vector<GLchar> infoLog(infoLogLength);
		glGetProgramInfoLog(m_id, (GLsizei)infoLog.size(), NULL, infoLog.data());

		printf("Shader link failed: %s", std::string(infoLog.begin(), infoLog.end()).c_str());
	}
	glDetachShader(m_id, computeShader.m_id);
}

GLProgram::~GLProgram()
{
	if (m_id != -1)
		glDeleteProgram(m_id);
}


GLTexture2D::GLTexture2D()
{
	glGenTextures(1, &tex_id);
}

GLTexture2D::~GLTexture2D()
{
	glDeleteTextures(1, &tex_id);
}


void GLTexture2D::load_memory_rgb(int width, int height, const uint8_t* data, bool is_srgb)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, is_srgb ? GL_SRGB : GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::load_memory_rgba(int width, int height, const uint8_t* data, bool is_srgb)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, is_srgb ? GL_SRGB_ALPHA : GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::load_memory_bgr(int width, int height, const uint8_t* data, bool is_srgb)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, is_srgb ? GL_SRGB : GL_RGBA, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void GLTexture2D::load_memory_bgra(int width, int height, const uint8_t* data, bool is_srgb)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, is_srgb ? GL_SRGB_ALPHA : GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::load_file(const char* filename, bool is_srgb)
{
	Image img;
	ImageLoader::LoadFile(&img, filename);
	load_memory_rgba(img.width(), img.height(), img.data(), is_srgb);
}

void GLTexture2D::unload()
{
	glDeleteTextures(1, &tex_id);
	glGenTextures(1, &tex_id);
}


GLTexture3D::GLTexture3D()
{
	glGenTextures(1, &tex_id);
}

GLTexture3D::~GLTexture3D()
{
	glDeleteTextures(1, &tex_id);
}

void GLTexture3D::load_memory(int width, int height, int depth, const uint8_t* data, int bytes_per_pixel)
{
	glBindTexture(GL_TEXTURE_3D, tex_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLenum type = GL_R8;
	if (bytes_per_pixel == 2) type = GL_R16;
	glTexImage3D(GL_TEXTURE_3D, 0, type, width, height, depth, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_3D, 0);

}


void GLTexture3D::unload()
{
	glDeleteTextures(1, &tex_id);
	glGenTextures(1, &tex_id);
}


GLCubemap::GLCubemap()
{
	glGenTextures(1, &tex_id);
}

GLCubemap::~GLCubemap()
{
	glDeleteTextures(1, &tex_id);
}

void GLCubemap::load_memory_rgba(int width, int height, const uint8_t* data_xp, const uint8_t* data_xn, const uint8_t* data_yp, const uint8_t* data_yn, const uint8_t* data_zp, const uint8_t* data_zn)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_xp);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_xn);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_yp);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_yn);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_zp);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_zn);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLCubemap::load_memory_rgbe(int width, int height, const float* data_xp, const float* data_xn, const float* data_yp, const float* data_yn, const float* data_zp, const float* data_zn)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB9_E5, width, height, 0, GL_RGB, GL_FLOAT, data_xp);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB9_E5, width, height, 0, GL_RGB, GL_FLOAT, data_xn);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB9_E5, width, height, 0, GL_RGB, GL_FLOAT, data_yp);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB9_E5, width, height, 0, GL_RGB, GL_FLOAT, data_yn);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB9_E5, width, height, 0, GL_RGB, GL_FLOAT, data_zp);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB9_E5, width, height, 0, GL_RGB, GL_FLOAT, data_zn);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLCubemap::load_files(const char* fn_xp, const char* fn_xn, const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn)
{
	Image imgs[6];
	ImageLoader::LoadFile(&imgs[0], fn_xp);
	ImageLoader::LoadFile(&imgs[1], fn_xn);
	ImageLoader::LoadFile(&imgs[2], fn_yp);
	ImageLoader::LoadFile(&imgs[3], fn_yn);
	ImageLoader::LoadFile(&imgs[4], fn_zp);
	ImageLoader::LoadFile(&imgs[5], fn_zn);
	load_memory_rgba(imgs[0].width(), imgs[0].height(), imgs[0].data(), imgs[1].data(), imgs[2].data(), imgs[3].data(), imgs[4].data(), imgs[5].data());
}

void GLCubemap::load_rgbe_files(const char* fn_xp, const char* fn_xn, const char* fn_yp, const char* fn_yn, const char* fn_zp, const char* fn_zn)
{
	HDRImage imgs[6];
	HDRImageLoader::LoadFile(&imgs[0], fn_xp);
	HDRImageLoader::LoadFile(&imgs[1], fn_xn);
	HDRImageLoader::LoadFile(&imgs[2], fn_yp);
	HDRImageLoader::LoadFile(&imgs[3], fn_yn);
	HDRImageLoader::LoadFile(&imgs[4], fn_zp);
	HDRImageLoader::LoadFile(&imgs[5], fn_zn);
	load_memory_rgbe(imgs[0].width(), imgs[0].height(), imgs[0].data(), imgs[1].data(), imgs[2].data(), imgs[3].data(), imgs[4].data(), imgs[5].data());

}

void GLCubemap::unload()
{
	glDeleteTextures(1, &tex_id);
	glGenTextures(1, &tex_id);
}

ReflectionMap::ReflectionMap()
{
	glGenTextures(1, &tex_id);
}

ReflectionMap::~ReflectionMap()
{
	glDeleteTextures(1, &tex_id);
}

void ReflectionMap::allocate()
{
	if (!allocated)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 7, GL_RGBA16F, 128, 128);
		allocated = true;
	}

}

GLBuffer::GLBuffer(size_t size, unsigned target)
{
	m_target = target;
	m_size = size;
	glGenBuffers(1, &m_id);
	glBindBuffer(m_target, m_id);
	glBufferData(m_target, m_size, nullptr, GL_STATIC_DRAW);
	glBindBuffer(m_target, 0);
}

GLBuffer::~GLBuffer()
{
	if (m_id != -1)
		glDeleteBuffers(1, &m_id);
}

void GLBuffer::upload(const void* data)
{
	glBindBuffer(m_target, m_id);
	glBufferData(m_target, m_size, data, GL_STATIC_DRAW);
	glBindBuffer(m_target, 0);
}

const GLBuffer& GLBuffer::operator = (const GLBuffer& in)
{
	glBindBuffer(GL_COPY_READ_BUFFER, in.m_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, m_id);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_size);
	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	return *this;
}


GLDynBuffer::GLDynBuffer(size_t size, unsigned target)
{
	m_target = target;
	m_size = size;
	glGenBuffers(1, &m_id);
	glBindBuffer(m_target, m_id);
	glBufferStorage(m_target, m_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(m_target, 0);
}

GLDynBuffer::~GLDynBuffer()
{
	if (m_id != -1)
	{
		glDeleteBuffers(1, &m_id);
	}
}

void GLDynBuffer::upload(const void* data)
{
	glBindBuffer(m_target, m_id);
	glBufferSubData(m_target, 0, m_size, data);	
	glBindBuffer(m_target, 0);
}

const GLDynBuffer& GLDynBuffer::operator = (const GLDynBuffer& in)
{
	glBindBuffer(GL_COPY_READ_BUFFER, in.m_id);
	glBindBuffer(GL_COPY_WRITE_BUFFER, m_id);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_size);
	glBindBuffer(GL_COPY_READ_BUFFER, 0);
	glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	return *this;
}

TextureBuffer::TextureBuffer(size_t size, unsigned internalFormat)
	: GLBuffer(size, GL_TEXTURE_BUFFER)
{
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_BUFFER, tex_id);
	glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, m_id);
}

TextureBuffer::~TextureBuffer()
{
	glDeleteTextures(1, &tex_id);
}

