cmake_minimum_required (VERSION 3.8)

project(signaler-examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../src ../src/function)
add_executable (examples "main.cpp")