#pragma once
#include <cstdint>

class AudioBuffer
{
public:
	AudioBuffer(int chn, int len);
	AudioBuffer(const AudioBuffer& in);
	~AudioBuffer();

	int chn() const { return m_chn; }
	int len() const { return m_len; }
	const uint8_t* data() const { return m_buffer; }
	uint8_t* data() { return m_buffer; }

	const uint8_t* get_data(int& chn, int& len) const;

	const AudioBuffer& operator=(const AudioBuffer& in);

private:
	int m_chn, m_len;
	uint8_t* m_buffer;

};
