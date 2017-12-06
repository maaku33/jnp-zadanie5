cmake_minimum_required(VERSION 3.8)
project(JNP5)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp lookup_error.h keyed_queue.h)
add_executable(JNP5 ${SOURCE_FILES})