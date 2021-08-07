#ifndef CHAT_ROOM_HPP
#define CHAT_ROOM_HPP

#include <string>
#include <set>
#include <deque>

#include "message.hpp"
#include "chatuser.hpp"

constexpr uint8_t max_cache_msgs = 100;

class ChatRoom {
    public:
        void join(chat_user_ptr user);
        void leave(chat_user_ptr user);
        void deliver(const Message& msg);
    private:
        std::set<chat_user_ptr> users_;
        std::deque<Message> msg_queue_;
};

#endif