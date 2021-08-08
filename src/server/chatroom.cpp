#include "chatroom.hpp"

void ChatRoom::join(chat_user_ptr user) {
    users_.insert(user);
    for (const auto& msg : msg_queue_)
        user->deliver(msg);
}

void ChatRoom::leave(chat_user_ptr user) {
    users_.erase(user);
}

void ChatRoom::deliverMsgToUsers(const Message& msg) {
    msg_queue_.push_back(msg);
    while (msg_queue_.size() > max_cache_msgs)
        msg_queue_.pop_front();
    for (auto user : users_)
        user->deliverMessageToClient(msg);
}