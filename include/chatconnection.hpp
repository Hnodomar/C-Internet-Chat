#ifndef CHAT_CONNECTION_HPP
#define CHAT_CONNECTION_HPP

#include <memory>
#include <boost/asio.hpp>

#include "chatroom.hpp"
#include "message.hpp"
#include "chatuser.hpp"

using boost::asio::ip::tcp;
class ChatConnection;
typedef std::enable_shared_from_this<ChatConnection> SharedConnection;

class ChatConnection : public SharedConnection, public ChatUser {
    public:
        ChatConnection(tcp::socket socket, ChatRoom& chatroom);
        void init();
        void deliverMsgToConnection(const Message& msg) override;
    private:
        void readMsgHeader();
        void readMsgBody();
        void writeMsgToClient();
        ChatRoom& chatroom_;
        tcp::socket socket_;
        Message temp_msg_;
        std::deque<Message> msgs_to_send_client_;
};

#endif