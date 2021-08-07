#include <iostream>
#include <memory>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Chat;

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
                    if (!ec)
                        std::make_shared<Chat>(std::move(socket), room_)->start();                
                    acceptConnections();
                }
            );
        }
        tcp::acceptor acceptor_;
        Chat room_;    
};

int main(int argc, char* argv[]) {
    try {
        if (argc > 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        boost::asio::io_context io_context;
        tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[1]));
        ChatServer(io_context, endpoint);
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }
    return 0;
}
