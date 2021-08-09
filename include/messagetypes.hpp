#ifndef MESSAGE_TYPES_HPP
#define MESSAGE_TYPES_HPP

#include "message.hpp"

struct JoinMessage : public Message {
    JoinMessage(const std::string& username):
        Message(
            std::string(username + std::string(" joined the room")), 
            (uint16_t)(username.length() + 19), 
            'M'
        ) 
    {}
};

struct LeaveMessage : public Message {
    LeaveMessage(const std::string& username):
        Message(
            std::string(username + std::string(" left the room")),
            (uint16_t)(username.length() + 14),
            'M'
        )
    {}
};

#endif