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
    uint8_t header[header_len + 1] = "";
    std::memcpy(header, packet_, header_len);
    body_len_ = ntohs(*header);
    if (body_len_ > max_body_len) {
        body_len_ = 0;
        return false;
    }
    return true;
}

void Message::setBodyLen(std::size_t new_len) {
    body_len_ = new_len;
    if (body_len_ > max_body_len)
        body_len_ = max_body_len;
}

void Message::addHeader() {
    uint8_t header[2];
    header[0] = body_len_ & 0xff;
    header[1] = (body_len_ >> 8);
    std::memcpy(packet_, header, header_len);
}
