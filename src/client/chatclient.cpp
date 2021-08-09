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
    Message msg_to_send;
    msg_to_send.setBodyLen(std::strlen(user_input) + username_.length());
    std::memcpy(
        msg_to_send.getMessagePacketBody(),
        username_.c_str(),
        username_.length()
    );
    std::memcpy(
        msg_to_send.getMessagePacketBody() + username_.length(), 
        user_input,
        msg_to_send.getMessagePacketBodyLen() 
    );
    msg_to_send.addHeader();
    addMsgToQueue(msg_to_send);    
}

void ChatClient::interpretCommand(const char* input) {
    std::string fullstr(input);
    uint16_t index = fullstr.find(' ');
    if (index == std::string::npos) {
        std::cout << "Invalid command" << std::endl;
        return;
    }
    std::string command = fullstr.substr(0, index);
    std::string argument = fullstr.substr(index + 1);
    if (command == "/nick")
        setClientNick(argument);
    else if (command == "/join")
        return;
    else 
        std::cout << "Invalid command" << std::endl;
}

void ChatClient::setClientNick(std::string& nick) {
    if (nick.length() > 10)
        std::cout << "Invalid nick: maximum length is 10 characters" << std::endl;
    else if (nick == "") 
        std::cout << "Invalid nick" << std::endl;
    else username_ = (nick + ": ");
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
            if (!ec && temp_msg_.parseHeader())
                readMsgBody();
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
                std::cout.write(
                    reinterpret_cast<const char*>(
                        temp_msg_.getMessagePacketBody()
                    ),
                    temp_msg_.getMessagePacketBodyLen()
                );
                std::cout << "\n";
                readMsgHeader();
            }
            else closeSocket();
        }
    );
}

void ChatClient::closeSocket() {
    boost::asio::post(io_context_, [this]() {
        socket_.close();
    });
}