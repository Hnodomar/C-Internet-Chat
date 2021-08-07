#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <set>
#include <algorithm>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "clientchatconnection.hpp"
#include "chatroom.hpp"

using boost::asio::ip::tcp;

class ChatServer : public std::enable_shared_from_this<ChatServer> {
    public:
        ChatServer();
    private:
        void acceptConnections();
        void getCRNameHeader(tcp::socket& socket) const;
        void getCRNameBody(tcp::socket& socket, uint16_t len) const;
        bool chatRoomExists(std::string name) const;
        tcp::acceptor acceptor_;
        std::set<ChatRoom> chatrooms_;
};

#endif