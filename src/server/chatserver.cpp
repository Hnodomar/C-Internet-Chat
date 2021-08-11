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
        logger_.write("[ SERVER ]: Successfully setup server");
        acceptConnections();
    }

void ChatServer::acceptConnections() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                logger_.write("[ SERVER ]: Client connected from "
                    + socket.remote_endpoint().address().to_string());
                std::make_shared<ChatConnection>(
                    std::move(socket), chatrooms_, logger_
                )->init();
            }                
            acceptConnections();
        }
    );
}
