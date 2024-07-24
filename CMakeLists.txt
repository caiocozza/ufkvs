cmake_minimum_required(VERSION 3.18)
project(ugkv C)

set(CMAKE_C_STANDARD 11)

add_executable(ugkv main.c
        server.c
        server.h
        epoll.h
        socket.h
        error.h)