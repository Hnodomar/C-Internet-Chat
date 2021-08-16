#include "chatroom.hpp"

ChatRoom::ChatRoom(std::string room_name)
    : room_name_(room_name) {}

void ChatRoom::join(chat_user_ptr user) {
    boost::asio::post(
        boost::asio::bind_executor(
            strand_,
            [user, this](){
                users_.insert(user);
                for (const auto& msg : msg_queue_)
                    user->deliverMsgToConnection(msg);
                deliverMsgToUsers(
                    JoinMessage(user->nick)
                );
            }
        )
    );
}

void ChatRoom::leave(chat_user_ptr user) {
    boost::asio::post(
        boost::asio::bind_executor(
            strand_,
            [user, this](){
                users_.erase(user);
                deliverMsgToUsers(
                    LeaveMessage(
                        user->nick
                    )
                );
            }
        )
    );
}

void ChatRoom::deliverMsgToUsers(const Message& msg) {
    boost::asio::post(
        boost::asio::bind_executor(
            strand_,
            [this, msg = std::move(msg)](){
                msg_queue_.push_back(msg);
                while (msg_queue_.size() > max_cache_msgs)
                    msg_queue_.pop_front();
                for (const auto& user : users_)
                    user->deliverMsgToConnection(msg);
            }
        )
    );
}

bool ChatRoom::nickAvailable(char* request_nick, uint16_t conn_id) {
    boost::asio::post(
        boost::asio::bind_executor(
            strand_,
            [this, request_nick, conn_id](){
                std::unique_lock<boost::fibers::mutex> nick_available_lock(nick_mtx);
                nick_available_finished = false;
                for (const auto& user : users_)
                    if (!strncmp(request_nick, user->nick, strlen(request_nick))) {
                        nick_available = false;
                        break;
                    }
                nick_available = true;
                latestID = conn_id;
                nick_available_finished = true;
                nick_cond.notify_all();
            }
        )
    );
}
