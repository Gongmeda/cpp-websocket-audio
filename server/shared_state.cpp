#include "shared_state.hpp"
#include "websocket_session.hpp"

void shared_state::join(websocket_session* session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
}

void shared_state::leave(websocket_session* session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
}

// Broadcast a message to all websocket client sessions
void shared_state::send(net::mutable_buffer message)
{
    // Put the message in a shared pointer so we can re-use it for each client
    auto const ss = std::make_shared<net::mutable_buffer const>(std::move(message));

    // Make a local list of all the weak pointers representing
    // the sessions, so we can do the actual sending without
    // holding the mutex:
    std::vector<std::weak_ptr<websocket_session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            v.emplace_back(p->weak_from_this());
    }

    // For each session in our local list, try to acquire a strong
    // pointer. If successful, then send the message on that session.
    for(auto const& wp : v)
        if(auto sp = wp.lock())
            sp->send(ss);
}
