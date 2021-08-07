#ifndef CHAT_CONNECTION_HPP
#define CHAT_CONNECTION_HPP

#include <memory>
#include <boost/asio.hpp>

#include "chatroom.hpp"
#include "message.hpp"

using boost::asio::ip::tcp;

class ChatConnection : public std::enable_shared_from_this<ChatConnection> {
    public:
        ChatConnection(tcp::socket socket, ChatRoom& chatroom);
        void init();
        void deliverMessageToClient();
    private:
        void readMsgHeader();
        void readMsgBody();
        ChatRoom& chatroom_;
        tcp::socket socket_;
        Message temp_msg_;
};

#endif