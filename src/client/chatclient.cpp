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
                std::cout << "[ CLIENT ] Successfully connected to " 
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
    auto removeUserInput = [](){std::cout << "\033[A" << "\33[2K";};
    while (std::cin.getline(user_input, (max_body_len - username_.length()) + 1)) {
        removeUserInput();
        if (user_input[0] == '/')
            interpretCommand(user_input);
        else if (room_name_.empty())
            std::cout << "[ CLIENT ] Please join a room first: /join <room>" << std::endl;
        else if (!username_.empty())
            constructMsg(user_input);
        else 
            std::cout << "[ CLIENT ] Please set username: /nick <user>" << std::endl;
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
            std::cout << "[ CLIENT ] Invalid command" << std::endl;
    }
    else std::cout << "[ CLIENT ] Invalid Command - still checking username" << std::endl;
}

void ChatClient::sendMsgToServerNoQueue(
    const std::string& body,
    char type,
    const std::string& success_msg,
    const std::string& fail_msg) {
    sendMsgToSocketNoQueue(
        body,
        type,
        [this,
        s=std::move(success_msg),
        f=std::move(fail_msg)](boost::system::error_code ec, std::size_t) {
            if (!ec)
                std::cout << s << std::endl;
            else {
                std::cout << f << std::endl;
                socket_.close();
            }
        },
        socket_
    );
}

void ChatClient::askServerToCreateRoom(std::string& roomname) {
    if (roomname.empty()) {
        std::cout << "[ CLIENT ] Invalid roomname" << std::endl;
        return;
    }
    sendMsgToServerNoQueue(
        roomname,
        'C',
        "[ CLIENT ] Requesting room creation by server",
        "[ CLIENT ] Failed to request room creation"
    );
}

void ChatClient::askServerForUserList() {
    if (!in_chatroom_) {
        std::cout << "[ CLIENT ] cannot fetch users list when not in a chatroom" << std::endl;
        return;
    }
    sendMsgToServerNoQueue(
        "",
        'U',
        "[ CLIENT ] Requesting user list from server",
        "[ CLIENT ] Failed to user list"
    );
}

void ChatClient::askServerForRoomList() {
    sendMsgToServerNoQueue(
        "",
        'L',
        "[ CLIENT ] Requesting room list from server",
        "[ CLIENT ] Failed to request room list"
    );
}

void ChatClient::setClientNick(std::string nick) {
    if (nick.length() > 10)
        std::cout << "[ CLIENT ] Invalid nick: maximum length is 10 characters" << std::endl;
    else if (nick == "") 
        std::cout << "[ CLIENT ] Invalid nick" << std::endl;
    else {
        username_temp_ = (nick + ": ");
        checking_username_ = true;
        sendMsgToSocketNoQueue(
            nick,
            'N',
            [this](boost::system::error_code ec, std::size_t a) {
                if (!ec) std::cout << "[ CLIENT ] Requesting nick from server" << std::endl;
                else {
                    std::cout << "[ CLIENT ] Failed to request nick from server" << std::endl;
                    checking_username_ = false;
                    username_temp_ = "";
                    socket_.close();
                }
            },
            socket_
        );
    }
}

void ChatClient::askServerToJoinRoom(std::string& roomname) {
    sendMsgToSocketNoQueue(
        roomname,
        'J',
        [this, roomname](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                std::cout << "[ CLIENT ] Requesting to join room " << roomname << std::endl;
                room_name_ = roomname;
            }
            else {
                std::cout << "[ CLIENT ] Failed to request join room" << std::endl;
                socket_.close();
            } 
        },
        socket_
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
            else socket_.close();
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
            else socket_.close();
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
            else socket_.close();
        }
    );
}

void ChatClient::handleCreateRoomMsg() {
    uint8_t notification = *(temp_msg_.getMessagePacketBody());
    switch(notification) {
        case 'Y':
            std::cout << "[ CLIENT ] Room successfully created!" << std::endl;
            break;
        case 'N':
            std::cout << "[ CLIENT ] Room failed to create!" << std::endl;
            break;
        default:
            break;
    }
}

void ChatClient::handleUserListMsg() {
    std::cout << "[ CLIENT ] User List:\n";
    formatAndOutputList(false);
}

void ChatClient::handleRoomListMsg() {
    std::cout << "[ CLIENT ] Room List:\n";
    formatAndOutputList(true);
}

void ChatClient::formatAndOutputList(bool is_room_list) {
     std::string list_str(
        reinterpret_cast<char*>(
            temp_msg_.getMessagePacketBody()
        ),
        temp_msg_.getMessagePacketBodyLen()
    );
    std::stringstream list_stream;
    list_stream << list_str;
    std::string temp;
    std::string user_temp = username_.substr(0, username_.length() - 2);
    while (getline(list_stream, temp, ' ')) {
        std::cout << "- " << temp;
        if (is_room_list && temp == room_name_)
            std::cout << " *";
        else if (!is_room_list && temp == user_temp)
            std::cout << " *";
        std::cout << std::endl;
    }
}

void ChatClient::handleChatMsg() {
    std::cout << getTimeString();
    std::cout << "[" + room_name_ + "] ";
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
        std::cout << "[ CLIENT ] Nick change success! Nick changed to: "
            << username_temp_.substr(0, username_temp_.length() - 2)
            << std::endl;
        username_ = username_temp_;
    }
    else
        std::cout << "[ CLIENT ] Nick taken! Please choose another" << std::endl;
    checking_username_ = false;
    username_temp_ = "";
}

void ChatClient::handleJoinMsg() {
    uint8_t join_response = *(temp_msg_.getMessagePacketBody());
    switch(join_response) {
        case 'Y':
            std::cout << "[ CLIENT ] Successfully joined room" << std::endl;
            in_chatroom_ = true;
            break;
        case 'N':
            std::cout << "[ CLIENT ] Cannot join room: doesn't exist" << std::endl;
            room_name_ = "";
            break;
        case 'U':
            std::cout << "[ CLIENT ] Cannot join room: nick in use" << std::endl;
            room_name_ = "";
            break;
        default:
            break;
    }
}
