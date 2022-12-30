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

class WSClientImpl : public WSClient::Impl
{
public:
	WSClientImpl(const char* host, const char* port)
		: m_resolver(m_ioc)
		, m_ws(m_ioc)
	{
		m_host = std::string(host) + ":" + port;

		m_resolver.async_resolve(
			host,
			port,
			std::bind(
				&WSClientImpl::on_resolve,
				this,
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void CheckPending() override
	{
		while (true)
		{
			int n = m_ioc.run_for(std::chrono::microseconds(1));
			if (n == 0) break;
		}
	}

	void Send(const void* data, size_t size, bool is_binary) override
	{
		m_ws.binary(is_binary);
		m_ws.async_write(
			net::buffer(data, size),
			std::bind(
				&WSClientImpl::on_write,
				this,
				std::placeholders::_1,
				std::placeholders::_2));
	}

private:
	net::io_context m_ioc;
	tcp::resolver m_resolver;

	// ws
	websocket::stream<tcp::socket> m_ws;

	beast::flat_buffer m_buffer;
	std::string m_host;

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

		if (open_callback != nullptr)
		{
			open_callback(open_callback_data);
		}
	}

	void on_read(boost::system::error_code ec,
		std::size_t bytes_transferred)
	{
		if (ec) return;

		if (msg_callback != nullptr)
		{
			const char* data = (const char*)m_buffer.data().data();
			size_t size = m_buffer.data().size();
			msg_callback(data, size, m_ws.got_binary(), msg_callback_data);
		}		

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

		m_resolver.async_resolve(
			host,
			port,
			std::bind(
				&WSClientImpl_SSL::on_resolve,
				this,
				std::placeholders::_1,
				std::placeholders::_2));
	}

	void CheckPending() override
	{
		while (true)
		{
			int n = m_ioc.run_for(std::chrono::microseconds(1));
			if (n == 0) break;
		}
	}

	void Send(const void* data, size_t size, bool is_binary) override
	{
		m_ws.binary(is_binary);
		m_ws.async_write(
			net::buffer(data, size),
			std::bind(
				&WSClientImpl_SSL::on_write,
				this,
				std::placeholders::_1,
				std::placeholders::_2));
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

		if (open_callback != nullptr)
		{
			open_callback(open_callback_data);
		}
	}

	void on_read(boost::system::error_code ec,
		std::size_t bytes_transferred)
	{
		if (ec) return;

		if (msg_callback != nullptr)
		{
			const char* data = (const char*)m_buffer.data().data();
			size_t size = m_buffer.data().size();
			msg_callback(data, size, m_ws.got_binary(), msg_callback_data);
		}

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

void WSClient::CheckPending()
{
	m_impl->CheckPending();
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