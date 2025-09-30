cmake_minimum_required(VERSION 3.16)
project(ultra_ipc CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# User-tunable flags
if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release)
endif()


add_compile_options(-O3 -march=native -fno-exceptions -fno-rtti -fno-plt)
add_link_options(-pthread)


include_directories(${CMAKE_SOURCE_DIR}/include)


add_library(ultraipc STATIC
src/dummy.cpp # placeholder to satisfy some generators; library is header-only otherwise
)


add_executable(ping src/ping.cpp)
target_link_libraries(ping ultraipc rt)


add_executable(pong src/pong.cpp)
target_link_libraries(pong ultraipc rt)