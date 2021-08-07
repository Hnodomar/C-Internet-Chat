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
                getChatRoomName(socket);
                std::make_shared<ClientChatConnection>(
                    std::move(socket), room_
                )->init();
            }                
            acceptConnections();
        }
    );
}

void ChatServer::getCRNameHeader(tcp::socket& socket) const {
    constexpr uint16_t header_len = 2;
    uint8_t header[header_len + 1] = "";
    auto self(shared_from_this());
    boost::asio::async_read(
        socket,
        boost::asio::buffer(header, header_len),
        [self, header, &socket](boost::system::error_code ec, std::size_t) {
            if(!ec) self->getCRNameBody(socket, ntohs(*header));
        }
    );
}

void ChatServer::getCRNameBody(tcp::socket& socket, const uint16_t len) const {
    auto self(shared_from_this());
    constexpr uint16_t body_len = 510;
    uint8_t chat_room_name[body_len + 1] = "";
    boost::asio::async_read(
        socket,
        boost::asio::buffer(chat_room_name, len),
        [self, chat_room_name, len](boost::system::error_code ec, std::size_t) {
            if (!ec) self->chatRoomExists(std::string(chat_room_name, chat_room_name + len));
        }
    );
}

bool ChatServer::chatRoomExists(std::string name) const {
    auto itr = std::find(chatrooms_.begin(), chatrooms_.end(), name);
    return itr != chatrooms_.end();
}