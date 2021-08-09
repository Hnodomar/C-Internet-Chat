#include "chatroom.hpp"

ChatRoom::ChatRoom(std::string room_name)
    : room_name_(room_name) {}

void ChatRoom::join(chat_user_ptr user) {
    users_.insert(user);
    for (const auto& msg : msg_queue_)
        user->deliverMsgToConnection(msg);
    deliverMsgToUsers(
        JoinMessage(user->nick)
    );
}

void ChatRoom::leave(chat_user_ptr user) {
    deliverMsgToUsers(
        LeaveMessage(user->nick)
    );
    users_.erase(user);
}

void ChatRoom::deliverMsgToUsers(const Message& msg) {
    msg_queue_.push_back(msg);
    while (msg_queue_.size() > max_cache_msgs)
        msg_queue_.pop_front();
    for (auto user : users_)
        user->deliverMsgToConnection(msg);
}

bool ChatRoom::nickAvailable(Message& msg, char* current_usernick) {
    bool nick_available = true;
    char nick_request[10] = "";
    memcpy(
        nick_request, 
        msg.getMessagePacketBody(), 
        msg.getMessagePacketBodyLen()
    );
    for (const auto& user : users_) {
        if (!strncmp(nick_request, user->nick, msg.getMessagePacketBodyLen()))
            nick_available = false;
    }
    if (nick_available) strncpy(current_usernick, nick_request, 10);
    return nick_available;
}