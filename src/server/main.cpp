#include <iostream>
#include <boost/asio.hpp>
#include <list>
#include "chatserver.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc > 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        boost::asio::io_context io_context;
        std::list<ChatServer> test;
        tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[1]));
        test.emplace_back(io_context, endpoint);
        io_context.run();
        std::cout << "running\n";
    }
    catch (std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }
    return 0;
}
