#ifndef CHAT_ROOM_HPP
#define CHAT_ROOM_HPP

#include <string>

class ChatRoom {
    public:
        ChatRoom();
        std::string getName() const;
    private:
        std::string name_;
};

#endif