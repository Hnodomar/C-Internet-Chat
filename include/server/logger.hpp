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
            outputstream(nullptr), strand_(std::move(strand)) {
            if (output_to_file) file.open("serverlog.txt", std::ios_base::app);
            outputstream = &(output_to_file ? file : std::cout);
            timefn_ptr = &(output_to_file ? getDateTimeString : getTimeString);
        }
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
            (*outputstream) << (*timefn_ptr)() << output << std::endl;
        }
        std::fstream file;
        std::ostream* outputstream;
        std::string (*timefn_ptr)() = nullptr;
        logger_strand strand_;
};

#endif
