project(oblivios_server_core)
file(GLOB_RECURSE SOURCE_FILES ./*.cpp)

add_library(oblivios_server_core STATIC ${SOURCE_FILES})