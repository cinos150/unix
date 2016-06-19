cmake_minimum_required(VERSION 3.0)
project(UnixProjectFinal)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall -Werror -pthread")

set(SOURCE_FILES server.c Utilities.c Utilities.h message_flow.c message_flow.h  structures.h LinkedList.c LinkedList.h  )
add_executable(UnixProjectFinal ${SOURCE_FILES})
add_executable(server server.c Utilities.c message_flow.c LinkedList.c )
