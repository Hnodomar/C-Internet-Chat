#include "message.hpp"

Message::Message() : body_len_(0) {}

const uint8_t* Message::getMessagePacket() const {
    return packet_;
}

const uint8_t* Message::getMessagePacketBody() const {
    return packet_ + header_len;
}

uint16_t Message::getMessagePacketBodyLen() const {
    return body_len_;
}