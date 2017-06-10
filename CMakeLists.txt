cmake_minimum_required(VERSION 3.7)
project(winscard_stub)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include /usr/include/PCSC)

set(SOURCE_FILES src/winscard_stub.cpp include/winscard_stub.h src/stubbing.cpp include/stubbing.h)

add_library(winscard_stub ${SOURCE_FILES})

set(TEST_SOURCE_FILES test/test_winscard_stub.cpp)

add_executable(winscard_test ${TEST_SOURCE_FILES})
target_link_libraries(winscard_test winscard_stub)