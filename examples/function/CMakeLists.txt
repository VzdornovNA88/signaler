cmake_minimum_required (VERSION 3.8)

set(SOURCES "../../src/function/function.cpp" "function_examples.cpp")

include_directories(../../src/function ../../src/detail ../../src/platforms ../../src/result)

add_executable (function-examples ${SOURCES})
