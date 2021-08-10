#include "util.hpp"

std::size_t findTimeStartIndex(const std::string& date_time, std::size_t pos, uint8_t iterations) {
    constexpr uint8_t date_pos = 2;
    std::size_t index = date_time.find(' ', pos);
    if (iterations == date_pos || index == std::string::npos) return index;
    return findTimeStartIndex(date_time, index + 1, iterations + 1);
}

std::string getTimeString() {
    std::time_t now = time(0);
    constexpr uint8_t time_len = 8;
    try {
        std::string date_time(ctime(&now));
        return std::string("[" + date_time.substr(
            findTimeStartIndex(date_time, 0, 0) + 1,
            time_len
        ) + "] ");
    }
    catch (std::exception& e) {
        return "00:00:00";
    }
}