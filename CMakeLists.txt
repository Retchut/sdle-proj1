cmake_minimum_required(VERSION 3.13)
project("SDLE1")
set(CMAKE_CXX_STANDARD 17)

FILE(GLOB INCLUDE_FILES src/Utils.cpp src/Topic.cpp src/Message.cpp)

# set(cppzmq_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cppzmq/build)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lzmq")

add_executable(Client src/Client.cpp ${INCLUDE_FILES})
add_executable(Server src/Server.cpp ${INCLUDE_FILES})

target_include_directories(Client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# find_package(cppzmq CONFIG REQUIRED)
# target_link_libraries(Client PRIVATE cppzmq cppzmq-static)
# target_link_libraries(Server PRIVATE cppzmq cppzmq-static)
