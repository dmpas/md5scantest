cmake_minimum_required(VERSION 3.10)
project(md5list)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(md5list main.cpp md5.h)

