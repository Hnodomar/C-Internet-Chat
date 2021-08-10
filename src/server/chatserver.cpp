#include "chatserver.hpp"

ChatServer::ChatServer(boost::asio::io_context& io_context,
    const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
    {
        chatrooms_.emplace(
            std::make_shared<ChatRoom>(
                "Lobby"
            )
        );
        acceptConnections();
    }

void ChatServer::acceptConnections() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "Client connected from " 
                    << socket.remote_endpoint().address().to_string()
                    << " \n";
                std::make_shared<ChatConnection>(
                    std::move(socket), chatrooms_
                )->init();
            }                
            acceptConnections();
        }
    );
}