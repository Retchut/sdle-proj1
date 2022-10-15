cmake_minimum_required(VERSION 3.13)  # CMake version check
project("SDLE1")               # Create project <Project_Name>
set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard


set(CLIENT_FILES client.cpp utils.cpp)
set(SERVER_FILES server.cpp utils.cpp)
set(cppzmq_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cppzmq/build)

add_executable(client ${CLIENT_FILES})
add_executable(server ${SERVER_FILES})

find_package(cppzmq CONFIG REQUIRED)
target_link_libraries(client PRIVATE cppzmq cppzmq-static)
target_link_libraries(server PRIVATE cppzmq cppzmq-static)
