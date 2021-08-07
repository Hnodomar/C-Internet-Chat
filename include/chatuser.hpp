#ifndef CHAT_USER_HPP
#define CHAT_USER_HPP

#include <memory>
#include "message.hpp"

typedef std::shared_ptr<ChatUser> chat_user_ptr;

class ChatUser {
    public:
        virtual ~ChatUser() {}
        virtual void deliverMessageToClient(const Message& msg) = 0;
};

#endif