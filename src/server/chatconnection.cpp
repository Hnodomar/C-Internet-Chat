#include "chatconnection.hpp"
#include <iostream>

ChatConnection::ChatConnection(tcp::socket socket, chatrooms& chat_rooms):
    socket_(std::move(socket)), chatrooms_set_(chat_rooms) {}

void ChatConnection::init() {
    sendClientRoomList(getChatroomNameList());
    readMsgHeader();
}

void ChatConnection::sendClientRoomList(const std::string& room_list) {
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            room_list,
            room_list.length()
        ),
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                std::cout << "Server: sent client chatroom list" << std::endl;
            }
        } 
    );
}

void ChatConnection::readMsgHeader() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(temp_msg_.getMessagePacket(), header_len),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec && temp_msg_.parseHeader())
                readMsgBody();
            else chatroom_->leave(self);
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
                switch(temp_msg_.type()) {
                    case 'M':
                        chatroom_->deliverMsgToUsers(temp_msg_);
                        break;
                    case 'N': {
                        char nick_available = chatroom_->nickAvailable(temp_msg_, nick) ? 'Y' : 'N';
                        notifyClientNickStatus(nick_available);
                        break;
                    }
                    case 'J':
                        handleJoinRoomMsg();
                        break;
                }
                readMsgHeader();
            }
            else chatroom_->leave(self);
        }
    );
}

void ChatConnection::handleJoinRoomMsg() {
    std::string room_name(
        reinterpret_cast<char*>(
            temp_msg_.getMessagePacketBody()
        ),
        temp_msg_.getMessagePacketBodyLen()   
    );
    auto chatroom_itr = getChatroomItrFromName(room_name);
    if (chatroom_itr != chatrooms_set_.end()) {
        (*chatroom_itr)->join(shared_from_this());
        joinRoomClientNotification('Y');
    }
    else {
        joinRoomClientNotification('N');
    }
}

void ChatConnection::joinRoomClientNotification(char success) {
    Message notification(
        std::string(1, success),
        1,
        'J'
    );
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            notification.getMessagePacket(),
            notification.getMsgPacketLen()
        ),
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec)
                std::cout << "Server: join room notification sent to client" << std::endl;
        }
    );
}

std::string ChatConnection::getChatroomNameList() const{
    std::string list;
    for (const auto& chatroom : chatrooms_set_) {
        list += chatroom->getRoomName();
        list += ' ';
    }
    return list;
}

chatrooms::iterator ChatConnection::getChatroomItrFromName(std::string& name) const {
    return std::find_if(
        chatrooms_set_.begin(),
        chatrooms_set_.end(),
        [&name](std::shared_ptr<ChatRoom> chatroom){
            return name == chatroom->getRoomName();
        }
    );
}

bool ChatConnection::chatroomNameExists(std::string& name) const {
    return !(getChatroomItrFromName(name) == chatrooms_set_.end());
}

void ChatConnection::notifyClientNickStatus(char nick_available) {
    Message nick_msg(std::string(1, nick_available), 1, 'N');
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
            else chatroom_->leave(self);
        }
    );
}