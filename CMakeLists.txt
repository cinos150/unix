cmake_minimum_required(VERSION 3.5)
project(UnixProjectFinal)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pthread")

set(SOURCE_FILES main.c)
add_executable(UnixProjectFinal ${SOURCE_FILES})
add_executable(main main.c )

