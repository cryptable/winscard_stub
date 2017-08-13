cmake_minimum_required(VERSION 3.5)
project(winscard_stub)

find_package( Threads )

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(${PROJECT_SOURCE_DIR}/include /usr/include/PCSC)

set(SOURCE_FILES src/winscard_stub.cpp include/winscard_stub.h src/stubbing.cpp include/stubbing.h include/missing_stl.h src/eventhandler/WinscardEventSubject.cpp src/eventhandler/WinscardEventSubject.h src/eventhandler/WinscardEventObserver.cpp src/eventhandler/WinscardEventObserver.h src/eventhandler/ReaderEventSubject.cpp src/eventhandler/ReaderEventSubject.h src/eventhandler/SmartcardEventSubject.cpp src/eventhandler/SmartcardEventSubject.h src/readers/SmartcardReader.cpp src/readers/SmartcardReader.h src/readers/PinpadReader.cpp src/readers/PinpadReader.h src/smartcards/Smartcard.cpp src/smartcards/Smartcard.h src/readers/NonPinpadReader.cpp src/readers/NonPinpadReader.h src/smartcards/TestSmartcard.cpp src/smartcards/TestSmartcard.h src/WinscardContext.cpp src/WinscardContext.h)

add_library(winscard_stub ${SOURCE_FILES})

set(TEST_SOURCE_FILES test/test_winscard_stub.cpp test/test_stubbing.cpp)

# Testing & Code Coverage support
enable_testing()
set(PROJECT_TEST_NAME winscard_test)
add_executable(${PROJECT_TEST_NAME} ${TEST_SOURCE_FILES})
set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_STANDARD 11)
target_link_libraries(${PROJECT_TEST_NAME} winscard_stub ${CMAKE_THREAD_LIBS_INIT})


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
    set_property(TARGET ${PROJECT_TEST_NAME}_coverage PROPERTY CXX_STANDARD 11)
endif()

add_test(test ${PROJECT_TEST_NAME})