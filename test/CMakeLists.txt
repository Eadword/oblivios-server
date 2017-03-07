project(oblivios_server_test)

set(GTEST_DIR lib/googletest/googletest)

include_directories(${GTEST_DIR}/include)
add_subdirectory(${GTEST_DIR})

file(GLOB SOURCE_FILES ./*.cpp)

add_executable(oblivios_server_tests ${SOURCE_FILES})

target_link_libraries(oblivios_server_tests gtest gtest_main)
target_link_libraries(oblivios_server_tests oblivios_server_core)