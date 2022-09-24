#include <cmath>
#include "AudioIO.h"
#include <pa_win_wasapi.h>

const std::vector<std::string>& AudioOut::s_get_list_audio_devices(bool refresh, int* id_default)
{
	static bool s_first_time = true;
	if (s_first_time)
	{
		Pa_Initialize();
		s_first_time = false;
	}

	static std::vector<std::string> s_list_devices;
	static int s_id_default;
	
	if (refresh || s_list_devices.size() == 0)
	{
		int numDevices;
		numDevices = Pa_GetDeviceCount();
		for (int i = 0; i < numDevices; i++)
		{
			const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
			s_list_devices.push_back(deviceInfo->name);
		}
		s_id_default = Pa_GetDefaultOutputDevice();
	}

	if (id_default != nullptr)
	{
		*id_default = s_id_default;
	}
	
	return s_list_devices;
}

int AudioOut::stream_callback(const void* inputBuffer, void* outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData)
{
	AudioOut* self = (AudioOut*)userData;
	short* out = (short*)outputBuffer;
	bool ret = self->m_callback(out, framesPerBuffer, self->m_user_ptr);
	if (ret)
	{
		return paContinue;
	}
	else
	{
		return paComplete;
	}
}

void AudioOut::stream_finished_callback(void* userData)
{
	AudioOut* self = (AudioOut*)userData;
	self->m_eof_callback(self->m_user_ptr);
}

AudioOut::AudioOut(int audio_device_id, int samplerate, AudioWriteCallback callback, EOFCallback eof_callback, void* user_ptr)
	: m_callback(callback), m_eof_callback(eof_callback), m_user_ptr(user_ptr)
{
	static bool s_first_time = true;
	if (s_first_time)
	{
		Pa_Initialize();
		s_first_time = false;
	}

	PaStreamParameters outputParameters = {};
	outputParameters.device = audio_device_id;
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;

	PaWasapiStreamInfo wasapi_info{0};
	wasapi_info.size = sizeof(PaWasapiStreamInfo);
	wasapi_info.hostApiType = paWASAPI;
	wasapi_info.version = 1;
	wasapi_info.flags = paWinWasapiAutoConvert;
	outputParameters.hostApiSpecificStreamInfo = &wasapi_info;
	
	unsigned long bufferFrameCount = (unsigned long)ceil(outputParameters.suggestedLatency * (double)samplerate);

	Pa_OpenStream(&m_stream, nullptr, &outputParameters, samplerate, bufferFrameCount, paClipOff, stream_callback, this);
	Pa_SetStreamFinishedCallback(m_stream, stream_finished_callback);
	Pa_StartStream(m_stream);

}

AudioOut::~AudioOut()
{
	Pa_CloseStream(m_stream);
}
