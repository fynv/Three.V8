#pragma once

#include <memory>
#include <string>
#include <vector>
#include <portaudio.h>

typedef bool(*AudioWriteCallback)(short* buf, size_t buf_size, void* user_ptr);
typedef void(*EOFCallback)(void* user_ptr);

namespace std
{
	class thread;
}

class AudioOut
{
public:
	AudioOut(int audio_device_id, int samplerate, AudioWriteCallback callback, EOFCallback eof_callback, void* user_ptr);
	~AudioOut();

	static const std::vector<std::string>& s_get_list_audio_devices(bool refresh, int* id_default);

private:
	AudioWriteCallback m_callback;
	EOFCallback m_eof_callback;
	void* m_user_ptr;

	PaStream* m_stream;

	static int stream_callback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData);

	static void stream_finished_callback(void* userData);


};
