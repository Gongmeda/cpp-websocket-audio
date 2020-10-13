#ifndef CPP_WS_ECHO_SERVER_WEBSOCKET_SESSION_HPP
#define CPP_WS_ECHO_SERVER_WEBSOCKET_SESSION_HPP

#include "header.hpp"
#include "shared_state.hpp"
#include <memory>
#include <queue>

// Forward declaration
class shared_state;

// Represents an active WebSocket connection to the server
class websocket_session : public std::enable_shared_from_this<websocket_session>
{
private:
    beast::flat_buffer buffer_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
    std::shared_ptr<shared_state> state_;
    std::queue<std::shared_ptr<net::mutable_buffer const>> queue_;

    void fail(beast::error_code ec, char const* what);
    void on_run();
    void on_handshake(beast::error_code ec);
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_send(std::shared_ptr<net::mutable_buffer const> const& ss);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

public:
    websocket_session(
        tcp::socket&& socket,
        ssl::context& ctx,
        std::shared_ptr<shared_state> const& state);

    ~websocket_session();

    void run();

    // Send a message
    void send(std::shared_ptr<net::mutable_buffer const> const& ss);
};

#endif
