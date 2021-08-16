#ifndef CHAT_ROOM_HPP
#define CHAT_ROOM_HPP

#include <string>
#include <set>
#include <deque>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/fiber/condition_variable.hpp>

#include "message.hpp"
#include "messagetypes.hpp"
#include "chatuser.hpp"

constexpr uint8_t max_cache_msgs = 100;
typedef boost::asio::strand<boost::asio::io_context::executor_type> chatroom_strand;

class ChatRoom {
    public:
        ChatRoom(const std::string room_name);
        void join(chat_user_ptr user);
        void leave(chat_user_ptr user);
        void deliverMsgToUsers(const Message& msg);
        bool nickAvailable(char* nick_request, uint16_t conn_id);
        std::string getRoomName() {return room_name_;}
        std::set<chat_user_ptr>& getUsers(){return users_;}
        boost::fibers::mutex nick_mtx;
        bool nick_available;
        bool nick_available_finished = false;
        uint16_t latestID = 0;
        boost::fibers::condition_variable nick_cond;
    private:
        const std::string room_name_;
        std::set<chat_user_ptr> users_;
        std::deque<Message> msg_queue_;
        chatroom_strand strand_;
};

#endif
