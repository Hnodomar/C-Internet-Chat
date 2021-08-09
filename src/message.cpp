#include "message.hpp"

Message::Message() : body_len_(0) {}

Message::Message(std::string& msg_body, uint16_t body_len, char type):
    body_len_(body_len), msg_type_(type) {
    addHeader(type);  
    memcpy(getMessagePacketBody(), msg_body.c_str(), body_len);      
}

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
    body_len_ = (header[0] & 0xff) |
        ((header[1] & 0xff) << 8);
    msg_type_ = header[2];
    if (msg_type_ != 'M' && msg_type_ != 'J' && msg_type_ != 'N')
        return false;
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

void Message::addHeader(char tag) {
    uint8_t header[header_len];
    header[0] = body_len_ & 0xff;
    header[1] = (body_len_ >> 8);
    header[2] = static_cast<uint8_t>(tag);
    std::memcpy(packet_, header, header_len);
}
