#include "chatroom.hpp"

ChatRoom::ChatRoom(std::string room_name)
    : room_name_(room_name) {}

void ChatRoom::join(chat_user_ptr user) {
    std::unique_lock<std::mutex> lock(chatroom_mutex_);
    users_.insert(user);
    for (const auto& msg : msg_queue_)
        user->writeMsgToClient(msg);
    lock.unlock();
    deliverMsgToUsers(
        JoinMessage(user->nick)
    );
}

void ChatRoom::leave(chat_user_ptr user) {
    std::unique_lock<std::mutex> lock(chatroom_mutex_);
    users_.erase(user);
    lock.unlock();
    deliverMsgToUsers(
        LeaveMessage(user->nick)
    );
}

void ChatRoom::deliverMsgToUsers(const Message& msg) {
    const std::lock_guard<std::mutex> lock(chatroom_mutex_);
    msg_queue_.push_back(msg);
    while (msg_queue_.size() > max_cache_msgs)
        msg_queue_.pop_front();
    for (const auto& user : users_)
        user->writeMsgToClient(msg);
}

bool ChatRoom::nickAvailable(char* request_nick) {
    const std::lock_guard<std::mutex> lock(chatroom_mutex_);
    for (const auto& user : users_)
        if (!strncmp(request_nick, user->nick, strlen(request_nick)))
            return false;
    return true;
}

std::string ChatRoom::getNicksList() {
    const std::lock_guard<std::mutex> lock(chatroom_mutex_);
    std::string list;
    for (const auto& user_ptr : users_) {
        list += user_ptr->nick;
        list += ' ';
    }
    list.pop_back();
    return std::move(list);
}
