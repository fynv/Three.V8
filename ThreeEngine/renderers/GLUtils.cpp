#include <GL/glew.h>
#include "utils/Utils.h"
#include "GLUtils.h"
#include <cstdio>
#include <vector>
#include <string>

#include "utils/Image.h"
#include "loaders/ImageLoader.h"

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

void GLTexture2D::load_memory_rgb(int width, int height, uint8_t* data, bool is_srgb)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, is_srgb ? GL_SRGB : GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::load_memory_rgba(int width, int height, uint8_t* data, bool is_srgb)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, is_srgb ? GL_SRGB_ALPHA : GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::load_memory_bgr(int width, int height, uint8_t* data, bool is_srgb)
{
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, is_srgb ? GL_SRGB : GL_RGBA, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture2D::load_file(const char* filename, bool is_srgb)
{
	Image img; 
	ImageLoader::LoadFile(&img, filename);
	load_memory_bgr(img.width(), img.height(), img.data(), is_srgb);
}
