cmake_minimum_required(VERSION 3.16)

project(Chat_Service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS iostreams system thread)

file(GLOB server_sources "src/server/*.cpp" "src/message.cpp" "src/util.cpp")
file(GLOB server_includes "include/server" "include")

add_executable(server ${server_sources})
target_include_directories(server PUBLIC ${server_includes})
target_link_libraries(server ${Boost_LIBRARIES})

file(GLOB client_sources "src/client/*.cpp" "src/message.cpp" "src/util.cpp")
file(GLOB client_includes "include/client" "include")

add_executable(client ${client_sources})
target_include_directories(client PUBLIC ${client_includes})
target_link_libraries(client ${Boost_LIBRARIES})

include_directories(${Boost_INCLUDE_DIR})
