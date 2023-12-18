#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

#include <string>
#include <vector>
#include <unordered_map>

// Get Async
struct GetResult
{
	bool result = false;
	std::vector<unsigned char> data;
};

typedef void (*GetCallback)(const GetResult& result, void* userData);

class HttpClient
{
public:
	HttpClient();
	~HttpClient();

	bool Get(const char* url, std::vector<unsigned char>& data);
	void GetAsync(const char* url, GetCallback callback, void* userData);

	bool GetHeaders(const char* url, std::unordered_map<std::string, std::string>& headers);
	bool GetRange(const char* url, size_t offset, size_t size, std::vector<unsigned char>& data);

private:
	net::io_context m_ioc;
	tcp::resolver m_resolver;

	// ssl
	ssl::context m_ssl_ctx;	

	// Get Async
	class GetData;
	static void GetThread(HttpClient* self, GetData* get_data);

};