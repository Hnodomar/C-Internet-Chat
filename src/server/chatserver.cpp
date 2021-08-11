#include "chatserver.hpp"

ChatServer::ChatServer(boost::asio::io_context& io_context,
    const tcp::endpoint& endpoint, bool output_to_file)
    : acceptor_(io_context, endpoint), logger_(output_to_file)
    {
        chatrooms_.emplace(
            std::make_shared<ChatRoom>(
                "Lobby"
            )
        );
        chatrooms_.emplace(
            std::make_shared<ChatRoom>(
                "Lobby2"
            )
        );
        chatrooms_.emplace(
            std::make_shared<ChatRoom>(
                "Lobby3"
            )
        );
        chatrooms_.emplace(
            std::make_shared<ChatRoom>(
                "Lobby4"
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
                    std::move(socket), chatrooms_, logger_
                )->init();
            }                
            acceptConnections();
        }
    );
}