cmake_minimum_required(VERSION 3.16)

project(Chat_Service)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS iostreams system thread)
file(GLOB server_sources "src/server/*.cpp" "src/message.cpp")
file(GLOB server_includes "include/server" "include")
file(GLOB client_sources "src/client/*.cpp" "src/message.cpp")
file(GLOB client_includes "include/client" "include")

include_directories(${Boost_INCLUDE_DIR} ${server_includes})
add_executable(server ${server_sources})
target_link_libraries(server ${Boost_LIBRARIES})