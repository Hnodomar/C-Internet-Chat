#include "chatconnection.hpp"
#include <iostream>

ChatConnection::ChatConnection(tcp::socket socket, ChatRoom& chatroom):
    socket_(std::move(socket)), chatroom_(chatroom) {}

void ChatConnection::init() {
    chatroom_.join(shared_from_this());
    readMsgHeader();
}

void ChatConnection::readMsgHeader() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(temp_msg_.getMessagePacket(), header_len),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec && temp_msg_.parseHeader())
                readMsgBody();
            else chatroom_.leave(shared_from_this());
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
                if (temp_msg_.type() == 'M')
                    chatroom_.deliverMsgToUsers(temp_msg_);
                else if (temp_msg_.type() == 'N') {
                    char nick_available = chatroom_.nickAvailable(temp_msg_, nick) ? 'Y' : 'N';
                    notifyClientNickStatus(nick_available);
                }
                //else if (temp_msg_.type() == 'J')
                readMsgHeader();
            }
            else chatroom_.leave(self);
        }
    );
}

void ChatConnection::notifyClientNickStatus(char nick_available) {
    Message nick_msg;
    uint16_t len = 1;
    nick_msg.setBodyLen(len);
    nick_msg.addHeader('N');
    *(nick_msg.getMessagePacketBody()) = nick_available;
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            nick_msg.getMessagePacket(),
            nick_msg.getMsgPacketLen()
        ),
        [this, nick_available](boost::system::error_code ec, std::size_t) {
            if (!ec)
                std::cout << "Server: sent nick status to client" << std::endl;
        }
    );
}

void ChatConnection::deliverMsgToConnection(const Message& msg) {
    bool already_delivering = !msgs_to_send_client_.empty();
    msgs_to_send_client_.push_back(msg);
    if (!already_delivering) writeMsgToClient();
}

void ChatConnection::writeMsgToClient() {
    auto self(shared_from_this());
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            msgs_to_send_client_.front().getMessagePacket(),
            msgs_to_send_client_.front().getMsgPacketLen() 
        ),
        [self, this](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                msgs_to_send_client_.pop_front();
                if (!msgs_to_send_client_.empty())
                    writeMsgToClient();
            }
            else chatroom_.leave(self);
        }
    );
}