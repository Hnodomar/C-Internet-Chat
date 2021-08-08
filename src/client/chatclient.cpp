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
            if (!ec) getMsgHeader();
        }
    );
    startInputLoop();
}

void ChatClient::startInputLoop() {
    boost::thread t([this](){
        io_context_.run(); //run async in one thread
    });                    //get input from another
    char user_input[max_body_len + 1];
    while (std::cin.getline(user_input, max_body_len + 1)) {
        Message msg_to_send;
        msg_to_send.setBodyLen(std::strlen(user_input));
        std::memcpy(
            msg_to_send.getMessagePacketBody(), 
            user_input,
            msg_to_send.getMessagePacketBodyLen() 
        );
        msg_to_send.addHeader();
        sendToServer(msg_to_send);
    }
    t.join();
}