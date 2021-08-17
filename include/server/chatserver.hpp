#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <set>
#include <algorithm>
#include <string>
#include <memory>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "chatconnection.hpp"
#include "chatroom.hpp"
#include "logger.hpp"

using boost::asio::ip::tcp;

class ChatServer {
    public:
        ChatServer(char* port, bool output_to_file);
        ~ChatServer();
    private:
        void acceptConnections();
        boost::asio::io_context io_context;
        tcp::acceptor acceptor_;
        boost::thread_group thrds_async;
        std::unique_ptr<boost::asio::io_context::work> work;
        std::set<std::shared_ptr<ChatRoom>> chatrooms_;
        Logger logger_;
};

#endif
