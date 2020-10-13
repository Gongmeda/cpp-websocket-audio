#include "websocket_session.hpp"
#include <iostream>

websocket_session::websocket_session(
    tcp::socket&& socket,
    ssl::context& ctx,
    std::shared_ptr<shared_state> const& state)
    : ws_(std::move(socket), ctx)
    , state_(state)
{
    ws_.binary(true);
}

websocket_session::~websocket_session()
{
    // Remove this session from the list of active sessions
    state_->leave(this);
}

void websocket_session::fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

void websocket_session::run()
{
    net::dispatch(ws_.get_executor(),
        beast::bind_front_handler(
            &websocket_session::on_run,
            shared_from_this()));
}

// Start the asynchronous operation
void websocket_session::on_run()
{
    std::cout << "on_run" << std::endl;

    // Set the timeout.
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        ssl::stream_base::server,
        beast::bind_front_handler(
            &websocket_session::on_handshake,
            shared_from_this()));
}

void websocket_session::on_handshake(beast::error_code ec)
{
    if (ec)
        return fail(ec, "handshake");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(http::field::server, "cpp-audio-server");   // Response Header 서버 이름
        }));

    std::cout << "on_handshake success" << std::endl;

    // Accept the websocket handshake
    ws_.async_accept(
        beast::bind_front_handler(
            &websocket_session::on_accept,
            shared_from_this()));
}

void websocket_session::on_accept(beast::error_code ec)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    state_->join(this);

    std::cout << "on_accept success" << std::endl;

    // Read a message
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &websocket_session::on_read,
            shared_from_this()));
}

void websocket_session::on_read(beast::error_code ec, std::size_t)
{
    // This indicates that the session was closed
    if (ec == websocket::error::closed)
        return;

    // Handle the error, if any
    if(ec)
        return fail(ec, "read");

    std::cout << "on_read success" << std::endl;

    // Send to all connections
    state_->send(buffer_.data());

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &websocket_session::on_read,
            shared_from_this()));
}

void websocket_session::send(std::shared_ptr<net::mutable_buffer const> const& ss)
{
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.

    std::cout << "send" << std::endl;

    net::post(
        ws_.get_executor(),
        beast::bind_front_handler(
            &websocket_session::on_send,
            shared_from_this(),
            ss));
}

void websocket_session::on_send(std::shared_ptr<net::mutable_buffer const> const& ss)
{
    // Always add to queue
    queue_.push(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    std::cout << "on_send success" << std::endl;

    // We are not currently writing, so send this immediately
    ws_.async_write(
        net::buffer(*queue_.front()),
        beast::bind_front_handler(
            &websocket_session::on_write,
            shared_from_this()));
}

void websocket_session::on_write(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.pop();

    std::cout << "on_write" << std::endl;

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(
            net::buffer(*queue_.front()),
            beast::bind_front_handler(
                &websocket_session::on_write,
                shared_from_this()));
}