#include "chatconnection.hpp"

ChatConnection::ChatConnection(tcp::socket socket, chatrooms& chat_rooms, Logger& logger):
    socket_(std::move(socket)), chatrooms_set_(chat_rooms), logger_(logger) {}

void ChatConnection::init() {
    readMsgHeader();
}

void ChatConnection::readMsgHeader() {
    auto self(shared_from_this());
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(temp_msg_.getMessagePacket(), header_len),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec && temp_msg_.parseHeader()) {
                readMsgBody();
            }
            else {
                if (chatroom_ != nullptr) 
                    chatroom_->leave(self);
            }
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
                        handleChatMsg();
                        break;
                    case 'N':
                        handleNickMsg();
                        break;
                    case 'J':
                        handleJoinRoomMsg();
                        break;
                    case 'L':
                        handleListRoomsMsg();
                        break;
                    case 'U':
                        handleListUsersMsg();
                        break;
                    case 'C':
                        handleCreateRoomMsg();
                        break;
                    default:
                        break;
                }
                readMsgHeader();
            }
            else {
                if (chatroom_ != nullptr)
                    chatroom_->leave(self);
            }
        }
    );
}

void ChatConnection::sendClientRoomList() {
    std::string room_names(getChatroomNameList());
    auto self = shared_from_this();
    sendMsgToSocketNoQueue(
        room_names,
        'L',
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                logger_.write("[ SERVER ]: Successfully sent client chatroom list");
            }
            else {
                logger_.write("[ SERVER ]: Failed to send client chatroom list");
                if (chatroom_ != nullptr)
                    chatroom_->leave(self);
            }
        },
        socket_
    );
}

void ChatConnection::handleCreateRoomMsg() {
    auto self = shared_from_this();
    std::string new_room_name(
        reinterpret_cast<char*>(
            temp_msg_.getMessagePacketBody()
        ),
        temp_msg_.getMessagePacketBodyLen()   
    );
    if (chatroomNameExists(new_room_name))
        sendMsgToSocketNoQueue(
            "N",
            'C',
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec)
                    logger_.write("[ SERVER ]: Notified client that room cannot be created: room exists");
                else {
                    logger_.write("[ SERVER ]: Failed to notify client that room cannot be created due to existence");
                    if (chatroom_ != nullptr)
                        chatroom_->leave(self);
                }
            },  
            socket_
        );
    else {
        sendMsgToSocketNoQueue(
            "Y",
            'C',
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec)
                    logger_.write("[ SERVER ]: Notified client that room can be created");
                else {
                    logger_.write("[ SERVER ]: Failed to notify client that room can be created");
                    if (chatroom_ != nullptr)
                        chatroom_->leave(self);
                }
            },  
            socket_
        );
        getChatrooms().emplace(
            std::make_shared<ChatRoom>(
                new_room_name
            )
        );
    }
}

void ChatConnection::handleListUsersMsg() {
    std::string user_list_str = getChatroomNicksList();
    auto self = shared_from_this();
    sendMsgToSocketNoQueue(
        user_list_str,
        'U',
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec)
                logger_.write("[ SERVER ]: Sent client user list");
            else {
                logger_.write("[ SERVER ]: Failed to send client user list");
                if (chatroom_ != nullptr)
                    chatroom_->leave(self);
            }
        },
        socket_
    );
}

void ChatConnection::handleListRoomsMsg() {
    sendClientRoomList();
}

void ChatConnection::handleChatMsg() {
    if (chatroom_ != nullptr)
        chatroom_->deliverMsgToUsers(temp_msg_);
    logger_.write(
        "[  USER  ] [" + chatroom_->getRoomName() + "]: " + 
        std::string(
            reinterpret_cast<char*>(
                temp_msg_.getMessagePacketBody()
            ),
            temp_msg_.getMessagePacketBodyLen()
        )
    );
}

void ChatConnection::handleNickMsg() {
    auto self = shared_from_this();
    char nick_request[10] = "";
    memcpy(
        nick_request, 
        temp_msg_.getMessagePacketBody(), 
        temp_msg_.getMessagePacketBodyLen()
    );
    bool nick_available = true;
    if (chatroom_ != nullptr) { //client is member of a chatroom
        nick_available = chatroom_->nickAvailable(nick_request);
        if (nick_available) 
            chatroom_->deliverMsgToUsers(
                NickChange(nick, nick_request)
            );
    }   
    if (nick_available) {
        strncpy(nick, nick_request, 10);
        sendMsgToSocketNoQueue(
            "Y",
            'N',
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec)
                    logger_.write("[ SERVER ]: Notified client that name change successful");
                else {
                    logger_.write("[ SERVER ]: Failed to notify client that name change successful");
                    if (chatroom_ != nullptr)
                        chatroom_->leave(self);
                }
            },
            socket_
        );
    }
    else {
        sendMsgToSocketNoQueue(
            "N",
            'N',
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec)
                    logger_.write("[ SERVER ]: Notified client that name change unsuccessful");
                else {
                    logger_.write("[ SERVER ]: Failed to notify client that name change unsuccessful");
                    if (chatroom_ != nullptr)
                        chatroom_->leave(self);
                }
            },
            socket_
        );
    }
}

void ChatConnection::handleJoinRoomMsg() {
    auto self = shared_from_this();
    std::string room_name(
        reinterpret_cast<char*>(
            temp_msg_.getMessagePacketBody()
        ),
        temp_msg_.getMessagePacketBodyLen()   
    );
    auto chatroom_itr = getChatroomItrFromName(room_name);
    if (chatroom_itr != chatrooms_set_.end()) {
        if ((*chatroom_itr)->nickAvailable(nick)) {
            if (chatroom_ != nullptr)
                chatroom_->leave(self);
            chatroom_ = (*chatroom_itr);
            sendMsgToSocketNoQueue( //room exists and nick not in use
                "Y",
                'J',
                [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec)
                        logger_.write("[ SERVER ]: Notified client that room join successful");
                    else {
                        logger_.write("[ SERVER ]: Failed to notify client that room join successful");
                        if (chatroom_ != nullptr)
                            chatroom_->leave(self);
                    }
                },
                socket_
            );
            (*chatroom_itr)->join(self);
        }
        else {
            sendMsgToSocketNoQueue( //room exists but nick in use
                "U",
                'J',
                [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec)
                        logger_.write("[ SERVER ]: Notified client that room join unsuccessful: name in use");
                    else {
                        logger_.write("[ SERVER ]: Failed to notify client that room join unsuccessful: name in use");
                        if (chatroom_ != nullptr)
                            chatroom_->leave(self);
                    }
                },
                socket_
            ); //room exists but nick in use
        }
    }
    else {
        sendMsgToSocketNoQueue( //room doesnt exist
            "N",
            'J',
            [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec)
                        logger_.write("[ SERVER ]: Notified client that room join unsuccessful: room doesnt exist");
                    else {
                        logger_.write("[ SERVER ]: Failed to notify client that room join unsuccessful: room doesnt exist");
                        if (chatroom_ != nullptr)
                            chatroom_->leave(self);
                    }
            },
            socket_
        );
    }
}

std::string ChatConnection::getChatroomNameList() const {
    std::string list;
    for (const auto& chatroom : chatrooms_set_) {
        list += chatroom->getRoomName();
        list += ' ';
    }
    list.pop_back();
    return list;
}

std::string ChatConnection::getChatroomNicksList() const {
    std::string list;
    for (const auto& user_ptr : chatroom_->getUsers()) {
        list += user_ptr->nick;
        list += ' ';
    }
    list.pop_back();
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
            else
                if (chatroom_ != nullptr) 
                    chatroom_->leave(self);
        }
    );
}
