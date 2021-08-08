#include "chatconnection.hpp"

ChatConnection::ChatConnection(tcp::socket socket, ChatRoom& chatroom):
    socket_(std::move(socket)), chatroom_(chatroom) {}

void ChatConnection::init() {
    room_.join(shared_from_this());
    readMsgHeader();
}

void ChatConnection::readMsgHeader() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(temp_msg_.getMessagePacket(), header_len),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec && self->temp_msg_.parseHeader()) readMsgBody();
            else self->chatroom_.leave(shared_from_this());
        }
    );
}

void ChatConnection::readMsgBody() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(
            temp_msg_.getMessagePacketBody(), 
            temp_msg_.getMessagePacketBodyLen()
        ),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                chatroom_.deliverMsgToUsers(temp_msg_);
                readMsgHeader();
            }
            else {
                chatroom_.leave(shared_from_this());
            }
        }
    );
}

void ChatConnection::deliverMessageToClient(const Message& msg) {
    
}