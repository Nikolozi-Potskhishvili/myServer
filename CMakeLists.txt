cmake_minimum_required(VERSION 3.28)
project(myServer C)

set(CMAKE_C_STANDARD 11)

add_executable(myServer main.c)
target_link_libraries(myServer ws2_32)