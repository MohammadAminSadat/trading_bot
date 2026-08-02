#include <boost/beast.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main()
{
    try
    {
        net::io_context ioc;

        // --- HTTP client: fetch a page ---
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        auto const results = resolver.resolve("httpbin.org", "80");
        stream.connect(results);

        http::request<http::string_body> req{http::verb::get, "/get", 11};
        req.set(http::field::host, "httpbin.org");
        req.set(http::field::user_agent, "Boost.Beast Example");

        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        std::cout << "HTTP/" << res.version() << ' '
                  << res.result_int() << ' '
                  << res.reason() << "\n\n";

        for (auto const& field : res)
            std::cout << field.name_string() << ": " << field.value() << '\n';

        std::cout << "\n--- body ---\n"
                  << res.body() << '\n';

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
