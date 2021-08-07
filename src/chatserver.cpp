#include "chatserver.hpp"

ChatServer::ChatServer(
                    boost::asio::io_context& io_context,
                    const tcp::endpoint& endpoint
                    ): acceptor_(io_context, endpoint)
                    {
                        acceptConnections();
                    }

void ChatServer::acceptConnections() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<ChatConnection>(
                    std::move(socket), room_
                )->init();
            }                
            acceptConnections();
        }
    );
}
