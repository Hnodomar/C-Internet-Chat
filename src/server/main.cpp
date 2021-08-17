#include <iostream>
#include <boost/asio.hpp>
#include <list>
#include "chatserver.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc > 3 || argc < 2) {
            std::cerr << "Usage: server <port> <logflag>\n";
            return 1;
        }
        bool output_to_file = false;
        if (argc > 2)
            output_to_file = !strcmp(argv[2], "-log") ? true : false;
        std::list<ChatServer> test;
        test.emplace_back(argv[1], output_to_file);
    }
    catch (std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }
    return 0;
}
