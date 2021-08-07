#ifndef CLIENT_CHAT_CONNECTION_HPP
#define CLIENT_CHAT_CONNECTION_HPP

#include <memory>

class ClientChatConnection : public std::enable_shared_from_this<ClientChatConnection> {
    public:
        ClientChatConnection();
        void init();
        void deliverMessageToClient();
    private:

};

#endif