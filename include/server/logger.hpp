#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include "util.hpp"

typedef boost::asio::strand<boost::asio::io_context::executor_type> logger_strand;

class Logger {
    public:
        Logger(bool output_to_file, logger_strand strand): 
            output_to_file(output_to_file), strand_(std::move(strand))
        {}
        void write(const std::string& output) {
            boost::asio::post(
                boost::asio::bind_executor(
                    strand_,
                    boost::bind(
                        &Logger::output, this, std::move(output)
                    )
                )
            );
        }
    private:
        void output(std::string output) { //all output is sequential
            if (output_to_file) {
                std::ofstream output_file;
                output_file.open("serverlog.txt", std::ios::app);
                output_file << getDateTimeString() << output << std::endl;
            }
            else 
                std::cout << output << std::endl;
        } 
        bool output_to_file;
        logger_strand strand_;
};

#endif
