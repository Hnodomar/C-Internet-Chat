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

struct NickChange : public Message {
    NickChange(const std::string& old_nick, const std::string& new_nick):
        Message(
            std::string(old_nick + std::string(" changed nick to " + new_nick)),
            (uint16_t)(old_nick.length() + 17 + new_nick.length()),
            'M'
        )
    {}
};

#endif