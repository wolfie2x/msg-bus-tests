cmake_minimum_required(VERSION 3.10)
project(mcast_demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

add_executable(publisher
    publisher.cpp MCTransport.cpp
)
target_include_directories(publisher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(subscriber
    subscriber.cpp MCTransport.cpp
)

target_include_directories(subscriber PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
