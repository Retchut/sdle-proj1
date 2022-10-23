cmake_minimum_required(VERSION 3.13)
project("SDLE1")
set(CMAKE_CXX_STANDARD 17)

FILE(GLOB INCLUDE_FILES src/Utils.cpp src/Topic.cpp src/Message.cpp)

add_executable(Client src/Client.cpp ${INCLUDE_FILES})
add_executable(Server src/Server.cpp ${INCLUDE_FILES})

target_link_libraries(Client PRIVATE zmq)
target_link_libraries(Server PRIVATE zmq)

