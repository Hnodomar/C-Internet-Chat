#ifndef CHAT_CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <string>

#include "message.hpp"

using boost::asio::ip::tcp;

class ChatClient {
    public:
        ChatClient(const tcp::resolver::results_type& endpoints, boost::asio::io_context& io_context);
        void initClient(const tcp::resolver::results_type& endpoints);
    private:
        void startInputLoop();
        void getMsgHeader();
        void getMsgBody();
        void sendToServer(Message& msg_to_send);
        tcp::socket socket_;
        boost::asio::io_context& io_context_;
};

#endif