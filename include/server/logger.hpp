#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include "util.hpp"

class Logger {
    public:
        Logger(bool output_to_file): 
            output_to_file(output_to_file)
        {}
        void write(const std::string& output) {
            if (output_to_file) {
                std::ofstream output_file;
                output_file.open("serverlog.txt", std::ios::app);
                output_file << getDateTimeString() << output << std::endl;
            }
            else 
                std::cout << output << std::endl;
        }
    private:
        bool output_to_file;
};

#endif
