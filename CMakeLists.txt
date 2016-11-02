cmake_minimum_required(VERSION 3.5)
project(oblivios_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(include)
include_directories(data)
include_directories(json/src)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_executable(oblivios_server ${SOURCE_FILES})