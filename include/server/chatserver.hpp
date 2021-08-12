#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <set>
#include <algorithm>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "chatconnection.hpp"
#include "chatroom.hpp"
#include "logger.hpp"

using boost::asio::ip::tcp;

class ChatServer {
    public:
        ChatServer(boost::asio::io_context& io_context,
                   const tcp::endpoint& endpoint,
                   bool output_to_file);
    private:
        void acceptConnections();
        tcp::acceptor acceptor_;
        std::set<std::shared_ptr<ChatRoom>> chatrooms_;
        Logger logger_;
};

#endif
