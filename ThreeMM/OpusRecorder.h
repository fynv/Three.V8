#pragma once

#include <memory>

typedef void (*PacketCallback)(const void* data, size_t size, void* ptr);

class OpusRecorder
{
public:
	OpusRecorder(int id_audio_device= -1);
	~OpusRecorder();

	void SetCallback(PacketCallback callback, void* callback_data);
	void* GetCallbackData();

private:
	class Internal;
	std::unique_ptr<Internal> m_internal;

};
