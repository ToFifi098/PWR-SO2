cmake_minimum_required(VERSION 3.22)
project(ChatProject)

set(CMAKE_CXX_STANDARD 17)

add_executable(chat_server server.cpp
        Semaphore.h)
target_link_libraries(chat_server ws2_32)

add_executable(chat_client client.cpp
        Semaphore.h)
target_link_libraries(chat_client ws2_32)

add_executable(chat_client_2 client.cpp
        Semaphore.h)
target_link_libraries(chat_client_2 ws2_32)