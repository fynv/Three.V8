#include "AudioBuffer.h"

extern "C" {
#include <libavformat/avformat.h>
}

#pragma comment(lib, "avformat.lib")

AudioBuffer::AudioBuffer(int chn, int len)
{
	m_chn = chn;
	m_len = len;
	int buf_size = av_samples_get_buffer_size(nullptr, chn, len, AV_SAMPLE_FMT_S16, 0);
	m_buffer = (uint8_t *)av_malloc(buf_size);
}

AudioBuffer::AudioBuffer(const AudioBuffer& in)
{
	m_chn = in.m_chn;
	m_len = in.m_len;
	int buf_size = av_samples_get_buffer_size(nullptr, m_chn, m_len, AV_SAMPLE_FMT_S16, 0);
	m_buffer = (uint8_t *)av_malloc(buf_size);
	memcpy(m_buffer, in.m_buffer, buf_size);
}

AudioBuffer::~AudioBuffer()
{
	av_free(m_buffer);
}


const uint8_t* AudioBuffer::get_data(int& chn, int& len) const
{
	chn = m_chn;
	len = m_len;
	return m_buffer;
}

const AudioBuffer& AudioBuffer::operator=(const AudioBuffer& in)
{
	int buf_size = av_samples_get_buffer_size(nullptr, m_chn, m_len, AV_SAMPLE_FMT_S16, 0);
	memcpy(m_buffer, in.m_buffer, buf_size);
	return *this;
}
