cmake_minimum_required(VERSION 3.13)
project(SK2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS "-pthread -std=c14 -Wall -Wextra -Wshadow")
set(CMAKE_CXX_FLAGS "-pthread -g -Wall")

set(SOURCE_FILES
        Lib.h
        Lib.cpp
        Node/ClientNode.cpp
        Node/ClientNode.h
        Node/Node.cpp
        Node/Node.h
        Node/ServerNode.cpp
        Node/ServerNode.h
        Messages/ComplexCommand.h
        Messages/ComplexCommand.cpp
        Messages/Command.h
        Messages/Command.cpp
        Messages/SimpleCommand.h
        Messages/SimpleCommand.cpp
        )

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#Here you can add another boost libriaries
find_package(Boost 1.45.0 COMPONENTS program_options)

add_executable(netstore-client client_main.cpp ${SOURCE_FILES})
target_link_libraries(netstore-client ${Boost_LIBRARIES})


add_executable(netstore-server server_main.cpp ${SOURCE_FILES})
target_link_libraries(netstore-server ${Boost_LIBRARIES})

