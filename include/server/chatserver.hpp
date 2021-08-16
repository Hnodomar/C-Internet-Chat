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
        ChatServer(const tcp::endpoint& endpoint, bool output_to_file);
        ~ChatServer();
    private:
        void acceptConnections();
        tcp::acceptor acceptor_;
        boost::thread_group thrds_async;
        boost::asio::io_context io_context;
        std::unique_ptr<boost::asio::io_context::work> work;
        std::set<std::shared_ptr<ChatRoom>> chatrooms_;
        Logger logger_;
        uint16_t conn_id_counter_ = 0;
        std::mutex id_counter_mutex_;
};

#endif
