#ifndef CPP_WS_ECHO_SERVER_SHARED_STATE_HPP
#define CPP_WS_ECHO_SERVER_SHARED_STATE_HPP

#include "header.hpp"
#include <memory>
#include <mutex>
#include <unordered_set>

// Forward declaration
class websocket_session;

// Represents the shared server state
class shared_state
{
private:
    // This mutex synchronizes all access to sessions_
    std::mutex mutex_;

    // Keep a list of all the connected clients
    std::unordered_set<websocket_session*> sessions_;

public:
    void join  (websocket_session* session);
    void leave (websocket_session* session);
    void send  (net::mutable_buffer message);
};

#endif
