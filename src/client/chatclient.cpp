#include "chatclient.hpp"

ChatClient::ChatClient(
    const tcp::resolver::results_type& endpoints, 
    boost::asio::io_context& io_context
    ): 
    socket_(io_context), io_context_(io_context) 
{
    initClient(endpoints);
}

void ChatClient::initClient(const tcp::resolver::results_type& endpoints) {
    boost::asio::async_connect(
        socket_,
        endpoints,
        [this](boost::system::error_code ec, tcp::endpoint) {
            if (!ec) {
                std::cout << "Successfully connected to " 
                    << socket_.remote_endpoint().address().to_string()
                    << std::endl;
                readMsgHeader();
            }
        }
    );
    startInputLoop();
}

void ChatClient::startInputLoop() {
    boost::thread t([this](){
        io_context_.run(); //run async recv in one thread
    });                    //get input from another
    char user_input[max_body_len + 1];
    while (std::cin.getline(user_input, (max_body_len - username_.length()) + 1)) {
        if (user_input[0] == '/')
            interpretCommand(user_input);
        else if (!username_.empty())
            constructMsg(user_input);
        else 
            std::cout << "Please set username: /nick <user>" << std::endl;
    }
    socket_.close();
    t.join();
}

void ChatClient::constructMsg(char* user_input) {
    std::string msg_body = username_ + std::string(user_input);
    Message msg_to_send(msg_body, msg_body.length(), 'M');
    addMsgToQueue(msg_to_send);    
}

CmdAndArg ChatClient::getCmdAndArg(const char* input) {
    std::string fullstr(input);
    std::size_t index = fullstr.find(' ');
    if (index == std::string::npos) {
        return {fullstr, ""};
    }
    std::string command = fullstr.substr(0, index);
    std::string argument = fullstr.substr(index + 1);
    return {command, argument};
}

void ChatClient::interpretCommand(const char* input) {
    auto [command, argument] = getCmdAndArg(input);
    if (!checking_username_) {
        if (command == "/nick")
            setClientNick(argument);
        else if (command == "/list")
            askServerForRoomList();
        else if (command == "/users")
            askServerForUserList();
        else if (command == "/create")
            askServerToCreateRoom(argument);
        else if (command == "/join" && !username_.empty())
            askServerToJoinRoom(argument);
        else 
            std::cout << "Invalid command" << std::endl;
    }
    else std::cout << "Invalid Command - still checking username" << std::endl;
}

void ChatClient::askServerToCreateRoom(std::string& roomname) {
    if (roomname.empty()) {
        std::cout << "Invalid roomname" << std::endl;
        return;
    }
    Message request_room_creation(
        roomname,
        roomname.length(),
        'C'
    );
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            request_room_creation.getMessagePacket(),
            request_room_creation.getMsgPacketLen()
        ),
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec)
                std::cout << "Requesting room creation by server" << std::endl; 
            else std::cout << "failed\n";
        }
    );  
}

void ChatClient::askServerForUserList() {
    if (!in_chatroom_) {
        std::cout << "Client: cannot fetch users list when not in a chatroom" << std::endl;
        return;
    }
    Message request_userlist(
        std::string(1, 'U'),
        1,
        'U'
    );
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            request_userlist.getMessagePacket(),
            request_userlist.getMsgPacketLen()
        ),
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec)
                std::cout << "Requesting user list from server" << std::endl; 
            else std::cout << "failed\n";
        }
    );  
}

void ChatClient::askServerForRoomList() {
    Message request_roomlist(
        std::string(1, 'L'),
        1,
        'L'
    );
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            request_roomlist.getMessagePacket(),
            request_roomlist.getMsgPacketLen()
        ),
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec)
                std::cout << "Requesting room list from server" << std::endl; 
            else std::cout << "failed\n";
        }
    );  
}

void ChatClient::setClientNick(std::string nick) {
    if (nick.length() > 10)
        std::cout << "Invalid nick: maximum length is 10 characters" << std::endl;
    else if (nick == "") 
        std::cout << "Invalid nick" << std::endl;
    else {
        username_temp_ = (nick + ": ");
        checking_username_ = true;
        Message check_user_msg(nick, nick.length(), 'N');
        boost::asio::async_write(
            socket_,
            boost::asio::buffer(
                check_user_msg.getMessagePacket(),
                check_user_msg.getMsgPacketLen()
            ),
            [this](boost::system::error_code ec, std::size_t a) {
                if (!ec) std::cout << "Requesting nick from server" << std::endl;
                else {
                    std::cout << "Failed to request nick from server" << std::endl;
                    checking_username_ = false;
                    username_temp_ = "";
                }
            }
        );
    }
}

void ChatClient::askServerToJoinRoom(std::string& roomname) {
    Message join_request(
        roomname,
        roomname.length(),
        'J'
    );
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            join_request.getMessagePacket(),
            join_request.getMsgPacketLen()  
        ),
        [this, roomname](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                std::cout << "Requesting to join room " << roomname << std::endl;
            }
        }
    );
}

void ChatClient::addMsgToQueue(const Message& msg) {
    boost::asio::post(io_context_, [this, msg]() {
        bool already_writing = !msg_queue_.empty();
        msg_queue_.push_back(msg);
        if (!already_writing) writeMsgToServer();
    });
}

void ChatClient::writeMsgToServer() {
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(
            msg_queue_.front().getMessagePacket(),
            msg_queue_.front().getMsgPacketLen()
        ),
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                msg_queue_.pop_front();
                if (!msg_queue_.empty()) writeMsgToServer();
            }
            else closeSocket();
        }
    );
}

void ChatClient::readMsgHeader() {
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(
            temp_msg_.getMessagePacket(),
            header_len
        ),
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec && temp_msg_.parseHeader()) {
                readMsgBody();
            }
            else closeSocket(); 
        }
    );
}

void ChatClient::readMsgBody() {
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(
            temp_msg_.getMessagePacketBody(),
            temp_msg_.getMessagePacketBodyLen()
        ),  
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                switch(temp_msg_.type()) {
                    case 'M':
                        handleChatMsg();
                        break;
                    case 'N':
                        handleNickMsg();
                        break;
                    case 'J':
                        handleJoinMsg();
                        break;
                    case 'L':
                        handleRoomListMsg();
                        break;
                    case 'U':
                        handleUserListMsg();
                        break;
                    case 'C':
                        handleCreateRoomMsg();
                        break;
                    default:
                        break;
                }
                readMsgHeader();
            }
            else closeSocket();
        }
    );
}

void ChatClient::handleCreateRoomMsg() {
    uint8_t notification = *(temp_msg_.getMessagePacketBody());
    switch(notification) {
        case 'Y':
            std::cout << "Room successfully created!" << std::endl;
            break;
        case 'N':
            std::cout << "Room failed to create!" << std::endl;
            break;
        default:
            break;
    }
}

void ChatClient::handleUserListMsg() {
    std::cout << "User List:\n";
    outputMsgBody();
}

void ChatClient::handleRoomListMsg() {
    std::cout << "Room List:\n";
    outputMsgBody();
}

void ChatClient::handleChatMsg() {
    outputMsgBody();
}

void ChatClient::outputMsgBody() {
    std::cout.write(
        reinterpret_cast<const char*>(
            temp_msg_.getMessagePacketBody()
        ),
        temp_msg_.getMessagePacketBodyLen()
    );
    std::cout << "\n";
}

void ChatClient::handleNickMsg() {
    uint8_t* nick_available = temp_msg_.getMessagePacketBody();
    if ((*nick_available) == 'Y') {
        std::cout << "Nick change success! Nick changed to: "
            << username_temp_.substr(0, username_temp_.length() - 2)
            << std::endl;
        username_ = username_temp_;
    }
    else
        std::cout << "Nick taken! Please choose another" << std::endl;
    checking_username_ = false;
    username_temp_ = "";
}

void ChatClient::handleJoinMsg() {
    uint8_t join_response = *(temp_msg_.getMessagePacketBody());
    switch(join_response) {
        case 'Y':
            std::cout << "Successfully joined room" << std::endl;
            in_chatroom_ = true;
            break;
        case 'N':
            std::cout << "Cannot join room: doesn't exist" << std::endl;
            break;
        case 'U':
            std::cout << "Cannot join room: nick in use" << std::endl;
            break;
        default:
            break;
    }
}

void ChatClient::closeSocket() {
    boost::asio::post(io_context_, [this]() {
        socket_.close();
    });
}