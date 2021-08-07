#ifndef MESSAGE_HPP
#define MESSAGE_HPP

constexpr uint16_t header_len = 2;
constexpr uint16_t max_body_len = 512;

#include <cstdint>

class Message {
    public:
        Message();
        const uint8_t* getMessagePacket() const;
        const uint8_t* getMessagePacketBody() const;
        uint16_t getMsgPacketLen() const;
    private:
        uint8_t packet_[header_len + max_body_len];
        uint16_t length_;
};

#endif