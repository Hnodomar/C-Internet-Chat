#ifndef CHAT_ROOM_HPP
#define CHAT_ROOM_HPP

#include <string>
#include <set>
#include <deque>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/fiber/condition_variable.hpp>

#include "message.hpp"
#include "messagetypes.hpp"
#include "chatuser.hpp"

constexpr uint8_t max_cache_msgs = 100;

class ChatRoom {
    public:
        ChatRoom(const std::string room_name);
        void join(chat_user_ptr user);
        void leave(chat_user_ptr user);
        void deliverMsgToUsers(const Message& msg);
        bool nickAvailable(char* nick_request);
        std::string getRoomName() {return room_name_;}
        std::set<chat_user_ptr>& getUsers(){return users_;}
    private:
        std::mutex chatroom_mutex_;
        const std::string room_name_;
        std::set<chat_user_ptr> users_;
        std::deque<Message> msg_queue_;
};

#endif
