#pragma once

#include <memory>

typedef void (*OpenCallback)(void* ptr);
typedef void (*MessageCallback)(const void* data, size_t size, bool is_binary, void* ptr);

class WSClient
{
public:
	WSClient(const char* url);
	~WSClient();

	class Impl
	{
	public:
		virtual void CheckPending() = 0;
		virtual void Send(const void* data, size_t size, bool is_binary) = 0;

		OpenCallback open_callback = nullptr;
		void* open_callback_data = nullptr;

		MessageCallback msg_callback = nullptr;
		void* msg_callback_data = nullptr;
	};

	void CheckPending();
	void Send(const void* data, size_t size, bool is_binary);

	void SetOpenCallback(OpenCallback open_callback, void* open_callback_data);
	void* GetOpenCallbackData();

	void SetMessageCallback(MessageCallback msg_callback, void* msg_callback_data);
	void* GetMessageCallbackData();

private:	
	std::unique_ptr<Impl> m_impl;
};

