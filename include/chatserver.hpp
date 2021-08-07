#ifndef CHAT_SERVER_HPP
#define CHAT_SERVER_HPP

#include <boost/asio.hpp>
#include "chat.hpp"
#include "chatroom.hpp"

using boost::asio::ip::tcp;

class ChatServer {
    public:
        ChatServer(
            boost::asio::io_context& io_context,
            const tcp::endpoint& endpoint
            ): acceptor_(io_context, endpoint)
        {
            acceptConnections();
        }  
    private:
        void acceptConnections() {
            acceptor_.async_accept(
                [this](boost::system::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::make_shared<ClientChatConnection>(
                            std::move(socket), room_
                        )->init();
                    }                
                    acceptConnections();
                }
            );
        }
        tcp::acceptor acceptor_;
        ChatRoom room_;    
};

#endif