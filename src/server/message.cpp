#include "message.hpp"

Message::Message() : body_len_(0) {}

uint8_t* Message::getMessagePacket() {
    return packet_;
}

uint8_t* Message::getMessagePacketBody() {
    return packet_ + header_len;
}

uint16_t Message::getMessagePacketBodyLen() const {
    return body_len_;
} 

uint16_t Message::getMsgPacketLen() const {
    return body_len_ + header_len;
}

bool Message::parseHeader() {
    return true;
}