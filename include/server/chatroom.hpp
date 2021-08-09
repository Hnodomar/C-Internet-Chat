#ifndef CHAT_ROOM_HPP
#define CHAT_ROOM_HPP

#include <string>
#include <set>
#include <deque>

#include "message.hpp"
#include "messagetypes.hpp"
#include "chatuser.hpp"

constexpr uint8_t max_cache_msgs = 100;

class ChatRoom {
    public:
        ChatRoom::ChatRoom(const std::string room_name);
        void join(chat_user_ptr user);
        void leave(chat_user_ptr user);
        void deliverMsgToUsers(const Message& msg);
        bool nickAvailable(Message& msg, char* current_usernick);
        std::string getRoomName() {return room_name_;}
        std::string getChatroomNameList();
    private:
        const std::string room_name_;
        std::set<chat_user_ptr> users_;
        std::deque<Message> msg_queue_;
};

#endif