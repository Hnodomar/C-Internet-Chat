#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <set>
#include <algorithm>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "chatconnection.hpp"
#include "chatroom.hpp"

using boost::asio::ip::tcp;

class ChatServer {
    public:
        ChatServer(boost::asio::io_context& io_context,
                   const tcp::endpoint& endpoint);
    private:
        void acceptConnections();
        tcp::acceptor acceptor_;
        ChatRoom chatroom_;
};

#endif