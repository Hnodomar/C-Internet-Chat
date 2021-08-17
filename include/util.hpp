#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <ctime>
#include <functional>
#include <boost/asio.hpp>
#include "message.hpp"

#ifdef THREADLOGGING
    #include <boost/thread/thread.hpp>
    #include <boost/lexical_cast.hpp>
    std::string getThreadIDString();
#endif

std::string getTimeString();
std::string getDateTimeString();
void sendMsgToSocketNoQueue(
    const std::string& body, 
    char tag, 
    const std::function<void(boost::system::error_code, std::size_t)>& handler,
    boost::asio::ip::tcp::socket& socket);
#endif
