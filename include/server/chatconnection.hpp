#ifndef CHAT_CONNECTION_HPP
#define CHAT_CONNECTION_HPP

#include <memory>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "chatroom.hpp"
#include "util.hpp"
#include "message.hpp"
#include "chatuser.hpp"
#include "logger.hpp"

using boost::asio::ip::tcp;
class ChatConnection;
typedef std::enable_shared_from_this<ChatConnection> SharedConnection;
typedef std::set<std::shared_ptr<ChatRoom>> chatrooms;
typedef boost::asio::strand<boost::asio::io_context::executor_type> connection_strand;

class ChatConnection : public SharedConnection, public ChatUser {
    public:
        ChatConnection(
            tcp::socket socket, 
            chatrooms& chatrooms, 
            Logger& logger,
            std::mutex& chatroom_set_mutex,
            std::string client_address_
        );
        void init();
        void writeMsgToClient(const Message& msg) override;
        chatrooms& getChatrooms() {return chatrooms_set_;}
    private:
        void readMsgHeader();
        void readMsgBody();
        void handleMsgBody();
        void handleChatMsg();
        void handleJoinRoomMsg();
        void handleNickMsg();
        void handleListRoomsMsg();
        void handleListUsersMsg();
        void handleCreateRoomMsg();
        void sendMsgToClientNoQueue(
            const std::string& success_msg, 
            const std::string& fail_msg,
            const std::string& body,
            char type
        );
        bool chatroomNameExists(std::string& name);
        std::string getChatroomNameList();
        std::string getChatroomNicksList() const;
        chatrooms::iterator getChatroomItrFromName(std::string& name);
        chatrooms& chatrooms_set_;
        const std::string client_address_;
        std::shared_ptr<ChatRoom> chatroom_;
        std::mutex& chatroom_set_mutex_;
        std::mutex msg_queue_mutex_;
        tcp::socket socket_;
        Message temp_msg_;
        Logger& logger_;
};

#endif
