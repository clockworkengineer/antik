cmake_minimum_required(VERSION 3.10.2)

project("Antik Unit Tests" VERSION 0.1.0 DESCRIPTION "Antik C++ Library Google Unit Tests")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wno-error=deprecated-declarations")

set(TEST_EXECUTABLE ${ANTIK_LIBRARY_NAME}_tests)

set(TEST_SOURCES
    UTCApprise.cpp
    UTCCCurl.cpp
    UTCCRedirect.cpp
    UTCFile.cpp
    UTCFileEventNotifier.cpp
    UTCFTP.cpp
    UTCIMAP.cpp
    UTCIMAPBodyStruct.cpp
    UTCIMAPParse.cpp
    UTCMIME.cpp
    UTCPath.cpp
    UTCSMTP.cpp
    UTCSocket.cpp
    UTCSSH.cpp
    UTCTask.cpp
    UTCUtility.cpp
    UTCZIP.cpp
    UTCZPIO.cpp
)

add_executable(${TEST_EXECUTABLE} ${TEST_SOURCES})
target_include_directories(${TEST_EXECUTABLE} PUBLIC ../include ../classes/implementation)

add_test(NAME ${TEST_EXECUTABLE} COMMAND ${TEST_EXECUTABLE})

target_link_libraries(${TEST_EXECUTABLE} PUBLIC gtest_main antik gtest gcov)