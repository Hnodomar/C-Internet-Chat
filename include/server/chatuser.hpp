#ifndef CHAT_USER_HPP
#define CHAT_USER_HPP

#include <memory>
#include "message.hpp"

class ChatUser {
    public:
        virtual ~ChatUser() {}
        virtual void writeMsgToClient(const Message& msg) = 0;
        char nick[10];
};

typedef std::shared_ptr<ChatUser> chat_user_ptr;

#endif
