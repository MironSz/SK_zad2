cmake_minimum_required(VERSION 3.13)
project(SK2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS "-pthread -std=c14 -Wall -Wextra -Wshadow")
set(CMAKE_CXX_FLAGS "-pthread -g -Wall")

#Google_tests MUST NOT be used in those files or project will not compile!!!
set(SOURCE_FILES
        Node/Lib.h
        Node/ClientNode.cpp
        Node/ClientNode.h
        Node/ServerNode.cpp
        Node/ServerNode.h
        Messages/ComplexCommand.cpp
        Messages/ComplexCommand.cpp
        Messages/SimpleCommand.h
        Messages/SimpleCommand.cpp
        )

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#Here you can add another boost libriaries
find_package(Boost 1.45.0 COMPONENTS program_options)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(client client_main.cpp ${SOURCE_FILES})
target_link_libraries(client ${Boost_LIBRARIES})

#Add Google_tests files here.
#set(TEST_FILES
#        unit_tests/test1.cpp
#        unit_tests/test2.cpp)
#
#add_subdirectory(Google_tests/lib)
#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
#add_executable(Google_Tests_run ${TEST_FILES} ${SOURCE_FILES})
#target_link_libraries(Google_Tests_run gtest gtest_main)
