cmake_minimum_required(VERSION 3.18)
project(ugkv C)

set(CMAKE_C_STANDARD 11)

find_package(Threads REQUIRED)

add_executable(ugkv main.c
        server.c
        server.h
        epoll.c
        epoll.h
        socket.h
        worker.h
        worker.c
        client.c
        client.h
        table.h
        table.c
        processor.h
        processor.c
        error.h)

target_link_libraries(ugkv PRIVATE Threads::Threads)