#pragma once

#include <memory>

typedef void (*PacketCallback)(const void* data, size_t size, void* ptr);


class AVCRecorder
{
public:
	AVCRecorder(int id_camera = 0);
	~AVCRecorder();

	void SetCallback(PacketCallback callback, void* callback_data);
	void* GetCallbackData();

private:
	class Internal;
	std::unique_ptr<Internal> m_internal;

};
