cmake_minimum_required(VERSION 3.10)
project(Sender C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lz")


add_executable(Sender main.c client.c server.c )