#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <cstring>
#include <arpa/inet.h>

constexpr uint16_t header_len = 3;
constexpr uint16_t max_body_len = 512;

class Message {
    public:
        Message();
        uint8_t* getMessagePacket();
        uint8_t* getMessagePacketBody();
        uint16_t getMessagePacketBodyLen() const;
        uint16_t getMsgPacketLen() const;
        void setBodyLen(std::size_t new_len); 
        bool parseHeader();
        void addHeader(char tag);
    private:
        uint8_t packet_[header_len + max_body_len];
        char msg_type_;
        uint16_t body_len_;
};

#endif
