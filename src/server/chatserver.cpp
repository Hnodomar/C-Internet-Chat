#include "chatserver.hpp"

ChatServer::ChatServer(char* port, bool output_to_file)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), std::atoi(port))), 
      logger_(output_to_file, std::move(boost::asio::make_strand(io_context)))
    {
        chatrooms_.emplace(
            std::make_shared<ChatRoom>(
                "Lobby"
            )
        );

        logger_.write("[ SERVER ]: Successfully setup server");
        work = std::make_unique<boost::asio::io_context::work>(io_context);
        for (int i = 0; i < boost::thread::hardware_concurrency() - 1; ++i)
            thrds_async.create_thread(
                boost::bind(&boost::asio::io_context::run, &io_context)
            );
        acceptConnections();
    }

ChatServer::~ChatServer() {
    work.reset();
    thrds_async.join_all();
}

void ChatServer::acceptConnections() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                logger_.write("[ SERVER ]: Client connected from "
                    + socket.remote_endpoint().address().to_string());
                std::make_shared<ChatConnection>(
                    std::move(socket), 
                    chatrooms_, 
                    logger_,
                    chatroom_set_mutex_
                )->init();
            }                
            acceptConnections();
        }
    );
}
