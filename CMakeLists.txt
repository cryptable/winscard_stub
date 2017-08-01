cmake_minimum_required(VERSION 3.5)
project(winscard_stub)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(${PROJECT_SOURCE_DIR}/include /usr/include/PCSC)

set(SOURCE_FILES src/winscard_stub.cpp include/winscard_stub.h src/stubbing.cpp include/stubbing.h)

add_library(winscard_stub ${SOURCE_FILES})

set(TEST_SOURCE_FILES test/test_winscard_stub.cpp test/test_stubbing.cpp)

# Testing & Code Coverage support
enable_testing()
set(PROJECT_TEST_NAME winscard_test)
add_executable(${PROJECT_TEST_NAME} ${TEST_SOURCE_FILES})
target_link_libraries(${PROJECT_TEST_NAME} winscard_stub)


if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(${PROJECT_TEST_NAME} gcov)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules)
if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    APPEND_COVERAGE_COMPILER_FLAGS()
    set(COVERAGE_EXCLUDES "test/catch.hpp" "5/*")
    setup_target_for_coverage(NAME ${PROJECT_TEST_NAME}_coverage
            EXECUTABLE ${PROJECT_TEST_NAME}
            DEPENDENCIES ${PROJECT_TEST_NAME})
endif()

add_test(test ${PROJECT_TEST_NAME})