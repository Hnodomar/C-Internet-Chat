cmake_minimum_required(VERSION 3.16)

project(Chat_Service)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS iostreams system thread)
file(GLOB SOURCES "src/*.cpp")

include_directories(${Boost_INCLUDE_DIR} include)
add_executable(server ${SOURCES})
target_link_libraries(server ${Boost_LIBRARIES})