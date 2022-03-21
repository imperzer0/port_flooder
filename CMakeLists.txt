cmake_minimum_required(VERSION 3.21)
project(port_flooder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_compile_definitions(VERSION="${PACKAGE_VERSION}")

add_executable(port_flooder main.cpp)
