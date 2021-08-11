#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <ctime>
#include <functional>
#include <boost/asio.hpp>
#include "message.hpp"

std::string getTimeString();
void sendMsgToSocketNoQueue(
    const std::string& body, 
    char tag, 
    const std::function<void(boost::system::error_code, std::size_t)>& handler,
    boost::asio::ip::tcp::socket& socket);
#endif