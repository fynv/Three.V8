#include <boost/url/src.hpp>
using namespace boost::urls;

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include "HttpClient.h"
#include <iostream>
#include <thread>

HttpClient::HttpClient()
	: m_resolver(m_ioc)
	, m_ssl_ctx(ssl::context::tlsv12_client)
{
	
	m_ssl_ctx.set_verify_mode(ssl::context::verify_peer);
	boost::certify::enable_native_https_server_verification(m_ssl_ctx);
}

HttpClient::~HttpClient()
{
	// Get
	{
		auto iter = m_pending_gets.begin();
		while (iter != m_pending_gets.end())
		{
			PendingGet* get_data = *iter;
			get_data->thread->join();
			delete get_data->thread;
			delete get_data;			
			iter++;
		}
	}
}


void HttpClient::CheckPendings()
{
	// Get
	{
		std::vector<PendingGet*> remove_lst;

		auto iter = m_pending_gets.begin();
		while (iter != m_pending_gets.end())
		{
			PendingGet* get_data = *iter;
			if (get_data->finished)
			{
				get_data->thread->join();
				delete get_data->thread;
				get_data->callback(get_data->result, get_data->userData);

				remove_lst.push_back(get_data);
			}
			iter++;
		}

		for (size_t i = 0; i < remove_lst.size(); i++)
		{
			delete remove_lst[i];
			m_pending_gets.erase(remove_lst[i]);
		}
	}
}

bool HttpClient::Get(const char* url, std::vector<unsigned char>& data)
{
	try
	{		
		url_view uv = parse_uri(url).value();

		auto scheme = uv.scheme();		
		if (scheme == "")
		{
			scheme = "http";
		}
		auto host = uv.host();
		auto target = uv.encoded_path();
		auto port = uv.port();
		if (port == "")
		{
			if (scheme == "https")
			{
				port = "443";
			}
			else
			{
				port = "80";
			}
		}
		if (target == "")
		{
			target = "/";
		}

		auto const results = m_resolver.resolve(host, port);

		if (scheme == "https")
		{
			beast::ssl_stream<beast::tcp_stream> stream(m_ioc, m_ssl_ctx);

			if (!SSL_set_tlsext_host_name(stream.native_handle(), std::string(host).c_str()))
			{
				return false;
			}

			beast::get_lowest_layer(stream).connect(results);
			stream.handshake(ssl::stream_base::client);
			http::request<http::string_body> req{ http::verb::get, std::string(target).c_str(), 11 };
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			for (auto seq : res.body().data())
			{
				size_t start = data.size();
				size_t end = start + seq.size();
				data.resize(end);
				memcpy(data.data() + start, seq.data(), seq.size());
			}

			beast::error_code ec;
			stream.shutdown(ec);
			if (ec)
			{
				return false;
			}
			return true;
		}
		else
		{
			beast::tcp_stream stream(m_ioc);
			stream.connect(results);

			http::request<http::string_body> req{ http::verb::get, std::string(target).c_str(), 11 };
			req.set(http::field::host, std::string(host).c_str());
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			for (auto seq : res.body().data())
			{
				size_t start = data.size();
				size_t end = start + seq.size();
				data.resize(end);
				memcpy(data.data() + start, seq.data(), seq.size());
			}

			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);
			if (ec)
			{
				return false;
			}
			return true;
		}
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
	return false;
}

void HttpClient::GetThread(HttpClient* self, PendingGet* get_data)
{
	get_data->result.result = self->Get(get_data->url.c_str(), get_data->result.data);
	get_data->finished = true;	
}

void HttpClient::GetAsync(const char* url, GetCallback callback, void* userData)
{
	PendingGet* get_data = new PendingGet;
	get_data->url = url;
	get_data->callback = callback;
	get_data->userData = userData;
	get_data->thread = new std::thread(GetThread, this, get_data);
	m_pending_gets.insert(get_data);
}

