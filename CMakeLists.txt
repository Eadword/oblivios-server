cmake_minimum_required(VERSION 3.5)
project(oblivios_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES main.cpp)

include_directories(src)
include_directories(data)

include_directories(lib/json/src)
include_directories(lib/base)

add_subdirectory(src)
add_subdirectory(test)

add_executable(oblivios_server ${SOURCE_FILES})
target_link_libraries(oblivios_server oblivios_server_core)