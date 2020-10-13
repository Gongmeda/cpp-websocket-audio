//-----------------------------------------------------------------------------------------------------------------------
/*
    < WebSocket Echo Server >

    This implements a multi-user binary echo server using WebSocket.
    Based on
    * https://www.boost.org/doc/libs/1_74_0/libs/beast/example/websocket/server/chat-multi/
    * https://www.boost.org/doc/libs/1_74_0/libs/beast/example/websocket/server/async-ssl/websocket_server_async_ssl.cpp
    
    Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)

    Copied & Modifed by. Gongmeda (https://github.com/Gongmeda)
*/
//-----------------------------------------------------------------------------------------------------------------------

#include "server_certificate.hpp"

#include "listener.hpp"
#include "shared_state.hpp"

#include <boost/asio/signal_set.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4) {
        std::cerr <<
            "Usage: websocket-chat-multi <address> <port> <threads>\n" <<
            "Example:\n" <<
            "    websocket-chat-server 0.0.0.0 8080 5\n";
        return EXIT_FAILURE;
    }
    auto address = net::ip::make_address(argv[1]);
    auto port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    net::io_context ioc{ threads };

    // The SSL context is required, and holds certificates
    ssl::context ctx{ ssl::context::tlsv12 };

    // This holds the self-signed certificate used by the server
    load_server_certificate(ctx);

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        ctx,
        tcp::endpoint{address, port},
        std::make_shared<shared_state>())->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&ioc](boost::system::error_code const&, int)
        {
            // Stop the io_context. This will cause run()
            // to return immediately, eventually destroying the
            // io_context and any remaining handlers in it.
            ioc.stop();
        });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for(auto& t : v)
        t.join();

    return EXIT_SUCCESS;
}
