#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <boost/beast/websocket.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>

#define BOOST_URL_SOURCE
#include <boost/url.hpp>
using namespace boost::urls;

#include "WSClient.h"
#include <iostream>

#include "root_certificates.hpp"

#include "utils/ConcurrentQueue.h"
#include "utils/AsyncCallbacks.h"

class WSClient::Impl
{
public:
	Impl()
	{
		AsyncCallbacks::AddSubQueue(&async_queue);
	}

	virtual ~Impl()
	{
		AsyncCallbacks::RemoveSubQueue(&async_queue);
	}

	virtual void Send(const void* data, size_t size, bool is_binary) = 0;

	OpenCallback open_callback = nullptr;
	void* open_callback_data = nullptr;

	MessageCallback msg_callback = nullptr;
	void* msg_callback_data = nullptr;

	struct Msg
	{
		std::vector<char> data;
		bool is_binary;
	};

	class OpenCallable : public Callable
	{
	public:
		Impl* impl;
		OpenCallable(Impl* impl) : impl(impl)
		{

		}

		void call() override
		{
			if (impl->open_callback != nullptr)
			{
				impl->open_callback(impl->open_callback_data);
			}
		}
	};

	class MessageCallable : public Callable
	{
	public:
		Impl* impl;
		Msg msg;
		MessageCallable(Impl* impl, const Msg& msg) : impl(impl), msg(msg)
		{


		}
		void call() override
		{
			if (impl->msg_callback != nullptr)
			{
				impl->msg_callback(msg.data.data(), msg.data.size(), msg.is_binary, impl->msg_callback_data);
			}
		}
	};

	AsyncQueue async_queue;
};

class WSClientImpl : public WSClient::Impl
{
public:
	WSClientImpl(const char* host, const char* port)
		: m_resolver(m_ioc)
		, m_ws(m_ioc)
	{
		m_host = std::string(host) + ":" + port;
		m_thread = (std::unique_ptr<std::thread>)(new std::thread(on_create, this, std::string(host), std::string(port)));
	}

	~WSClientImpl()
	{
		m_running = false;
		m_thread->join();
	}	

	void Send(const void* data, size_t size, bool is_binary) override
	{
		std::vector<char> msg_data(size);
		memcpy(msg_data.data(), data, size);
		m_user_write_queue.Push({ msg_data, is_binary });
	}

private:
	net::io_context m_ioc;
	tcp::resolver m_resolver;

	// ws
	websocket::stream<tcp::socket> m_ws;

	beast::flat_buffer m_buffer;
	std::string m_host;
	
	std::queue<Msg> m_write_queue;
	ConcurrentQueue<Msg> m_user_write_queue;

	bool m_running = true;	
	std::unique_ptr<std::thread> m_thread;

	static void on_create(WSClientImpl* self, std::string host, std::string port)
	{		
		self->m_resolver.async_resolve(
			host,
			port,
			std::bind(
				&WSClientImpl::on_resolve,
				self,
				std::placeholders::_1,
				std::placeholders::_2));

		while (self->m_running)
		{
			while (self->m_user_write_queue.Size() > 0)
			{
				Msg msg = self->m_user_write_queue.Pop();
				self->m_write_queue.push(msg);

				if (self->m_write_queue.size() > 1) continue;

				self->m_ws.binary(msg.is_binary);
				self->m_ws.async_write(
					net::buffer(msg.data.data(), msg.data.size()),
					std::bind(
						&WSClientImpl::on_write,
						self,
						std::placeholders::_1,
						std::placeholders::_2));
			}

			while (true)
			{
				int n = self->m_ioc.run_for(std::chrono::microseconds(1));
				if (n == 0) break;
			}
		}
	}

	void on_resolve(boost::system::error_code ec, tcp::resolver::results_type results)
	{
		net::async_connect(
			m_ws.next_layer(),
			results.begin(),
			results.end(),
			std::bind(
				&WSClientImpl::on_connect,
				this,
				std::placeholders::_1));
	}

	void on_connect(boost::system::error_code ec)
	{
		m_ws.async_handshake(m_host, "/",
			std::bind(
				&WSClientImpl::on_handshake,
				this,
				std::placeholders::_1));
	}

	void on_handshake(boost::system::error_code ec)
	{
		m_ws.async_read(
			m_buffer,
			std::bind(
				&WSClientImpl::on_read,
				this,
				std::placeholders::_1,
				std::placeholders::_2));

		async_queue.Add(new OpenCallable(this));
	}

	void on_read(boost::system::error_code ec,
		std::size_t bytes_transferred)
	{
		if (ec) return;

		Msg msg;
		msg.data.resize(m_buffer.data().size());
		memcpy(msg.data.data(), m_buffer.data().data(), m_buffer.data().size());
		msg.is_binary = m_ws.got_binary();

		async_queue.Add(new MessageCallable(this, msg));

		m_buffer.clear();

		m_ws.async_read(
			m_buffer,
			std::bind(
				&WSClientImpl::on_read,
				this,
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void on_write(boost::system::error_code ec,
		std::size_t bytes_transferred)
	{
		m_write_queue.pop();
		if (!m_write_queue.empty())
		{
			Msg msg = m_write_queue.front();

			m_ws.binary(msg.is_binary);
			m_ws.async_write(
				net::buffer(msg.data.data(), msg.data.size()),
				std::bind(
					&WSClientImpl::on_write,
					this,
					std::placeholders::_1,
					std::placeholders::_2));
		}
	}

};

class WSClientImpl_SSL : public WSClient::Impl
{
public:
	WSClientImpl_SSL(const char* host, const char* port)
		: m_resolver(m_ioc)
		, m_ssl_ctx(ssl::context::tlsv12_client)
		, m_ws(m_ioc, m_ssl_ctx)
	{
		load_root_certificates(m_ssl_ctx);
		m_ssl_ctx.set_verify_mode(ssl::context::verify_peer);

		m_host = std::string(host) + ":" + port;
		m_thread = (std::unique_ptr<std::thread>)(new std::thread(on_create, this, std::string(host), std::string(port)));

	}

	~WSClientImpl_SSL()
	{
		m_running = false;
		m_thread->join();
	}

	void Send(const void* data, size_t size, bool is_binary) override
	{
		std::vector<char> msg_data(size);
		memcpy(msg_data.data(), data, size);
		m_user_write_queue.Push({ msg_data, is_binary });
	}

private:
	net::io_context m_ioc;
	tcp::resolver m_resolver;

	// ssl
	ssl::context m_ssl_ctx;

	// ws
	websocket::stream<ssl::stream<tcp::socket>> m_ws;

	beast::flat_buffer m_buffer;
	std::string m_host;

	std::queue<Msg> m_write_queue;
	ConcurrentQueue<Msg> m_user_write_queue;

	bool m_running = true;	
	std::unique_ptr<std::thread> m_thread;

	static void on_create(WSClientImpl_SSL* self, std::string host, std::string port)
	{
		self->m_resolver.async_resolve(
			host,
			port,
			std::bind(
				&WSClientImpl_SSL::on_resolve,
				self,
				std::placeholders::_1,
				std::placeholders::_2));

		while (self->m_running)
		{
			while (self->m_user_write_queue.Size() > 0)
			{
				Msg msg = self->m_user_write_queue.Pop();
				self->m_write_queue.push(msg);

				if (self->m_write_queue.size() > 1) continue;

				self->m_ws.binary(msg.is_binary);
				self->m_ws.async_write(
					net::buffer(msg.data.data(), msg.data.size()),
					std::bind(
						&WSClientImpl_SSL::on_write,
						self,
						std::placeholders::_1,
						std::placeholders::_2));
			}

			while (true)
			{
				int n = self->m_ioc.run_for(std::chrono::microseconds(1));
				if (n == 0) break;
			}
		}
	}

	void on_resolve(boost::system::error_code ec, tcp::resolver::results_type results)
	{
		net::async_connect(
			m_ws.next_layer().next_layer(),
			results.begin(),
			results.end(),
			std::bind(
				&WSClientImpl_SSL::on_connect,
				this,
				std::placeholders::_1));
	}

	void on_connect(boost::system::error_code ec)
	{
		m_ws.next_layer().async_handshake(
			ssl::stream_base::client,			
			std::bind(
				&WSClientImpl_SSL::on_ssl_handshake,
				this,
				std::placeholders::_1));
	}

	void on_ssl_handshake(boost::system::error_code ec)
	{
		m_ws.async_handshake(m_host, "/",
			std::bind(
				&WSClientImpl_SSL::on_handshake,
				this,
				std::placeholders::_1));
	}

	void on_handshake(boost::system::error_code ec)
	{
		m_ws.async_read(
			m_buffer,
			std::bind(
				&WSClientImpl_SSL::on_read,
				this,
				std::placeholders::_1,
				std::placeholders::_2));

		async_queue.Add(new OpenCallable(this));
	}

	void on_read(boost::system::error_code ec,
		std::size_t bytes_transferred)
	{
		if (ec) return;

		Msg msg;
		msg.data.resize(m_buffer.data().size());
		memcpy(msg.data.data(), m_buffer.data().data(), m_buffer.data().size());
		msg.is_binary = m_ws.got_binary();

		async_queue.Add(new MessageCallable(this, msg));

		m_buffer.clear();

		m_ws.async_read(
			m_buffer,
			std::bind(
				&WSClientImpl_SSL::on_read,
				this,
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void on_write(boost::system::error_code ec,
		std::size_t bytes_transferred)
	{
		m_write_queue.pop();
		if (!m_write_queue.empty())
		{
			Msg msg = m_write_queue.front();

			m_ws.binary(msg.is_binary);
			m_ws.async_write(
				net::buffer(msg.data.data(), msg.data.size()),
				std::bind(
					&WSClientImpl_SSL::on_write,
					this,
					std::placeholders::_1,
					std::placeholders::_2));
		}
	}
};

WSClient::WSClient(const char* url)	
{	
	url_view uv = parse_uri(url).value();

	auto scheme = uv.scheme();
	if (scheme == "")
	{
		scheme = "ws";
	}
	auto host = uv.host();		
	auto port = uv.port();
	if (port == "")
	{
		if (scheme == "wss")
		{
			port = "443";
		}
		else
		{
			port = "80";
		}
	}
		
	if (scheme == "ws")
	{
		m_impl = std::unique_ptr<Impl>(new WSClientImpl(host.c_str(), std::string(port).c_str()));
	}
	else if (scheme == "wss")
	{
		m_impl = std::unique_ptr<Impl>(new WSClientImpl_SSL(host.c_str(), std::string(port).c_str()));
	}
}

WSClient::~WSClient()
{


}

void WSClient::Send(const void* data, size_t size, bool is_binary)
{
	m_impl->Send(data, size, is_binary);
}

void WSClient::SetOpenCallback(OpenCallback open_callback, void* open_callback_data)
{
	m_impl->open_callback = open_callback;
	m_impl->open_callback_data = open_callback_data;
}

void* WSClient::GetOpenCallbackData()
{
	return m_impl->open_callback_data;
}

void WSClient::SetMessageCallback(MessageCallback msg_callback, void* msg_callback_data)
{
	m_impl->msg_callback = msg_callback;
	m_impl->msg_callback_data = msg_callback_data;

}

void* WSClient::GetMessageCallbackData()
{
	return m_impl->msg_callback_data;
}