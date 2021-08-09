#ifndef CHAT_CONNECTION_HPP
#define CHAT_CONNECTION_HPP

#include <memory>
#include <boost/asio.hpp>

#include "chatroom.hpp"
#include "message.hpp"
#include "chatuser.hpp"

using boost::asio::ip::tcp;
class ChatConnection;
typedef std::enable_shared_from_this<ChatConnection> SharedConnection;
typedef std::set<std::shared_ptr<ChatRoom>> chatrooms;

class ChatConnection : public SharedConnection, public ChatUser {
    public:
        ChatConnection(tcp::socket socket, chatrooms& chatrooms);
        void init();
        void deliverMsgToConnection(const Message& msg) override;
    private:
        void readMsgHeader();
        void readMsgBody();
        void writeMsgToClient();
        void notifyClientNickStatus(char nick_available);
        void handleJoinRoomMsg();
        void joinRoomClientNotification(char success);
        void sendClientRoomList(std::string& roomlist);
        bool chatroomNameExists(std::string& name) const;
        std::string getChatroomNameList() const;
        chatrooms::iterator getChatroomItrFromName(std::string& name) const;
        chatrooms& chatrooms_set_;
        std::shared_ptr<ChatRoom> chatroom_;
        tcp::socket socket_;
        Message temp_msg_;
        std::deque<Message> msgs_to_send_client_;
};

#endif
