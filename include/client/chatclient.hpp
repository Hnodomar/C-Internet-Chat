#ifndef CHAT_CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <string>
#include <tuple>
#include <deque>

#include "message.hpp"

using boost::asio::ip::tcp;
typedef std::tuple<std::string, std::string> CmdAndArg;

class ChatClient {
    public:
        ChatClient(const tcp::resolver::results_type& endpoints, boost::asio::io_context& io_context);
        void initClient(const tcp::resolver::results_type& endpoints);
    private:
        void startInputLoop();
        void constructMsg(char* user_input);
        void writeUsernameToMsgBody(Message& msg, std::string& username);
        void interpretCommand(const char* input);
        void setClientNick(std::string nick);
        void readMsgHeader();
        void readMsgBody();
        void addMsgToQueue(const Message& msg);
        void writeMsgToServer();
        void closeSocket();
        void askServerToJoinRoom(std::string& roomname);
        CmdAndArg getCmdAndArg(const char* input);

        void handleChatMsg();
        void handleNickMsg();
        void handleJoinMsg();

        tcp::socket socket_;
        std::string username_;
        std::string username_temp_;
        bool checking_username_ = false;
        boost::asio::io_context& io_context_;
        Message temp_msg_;
        std::deque<Message> msg_queue_;
};

#endif
