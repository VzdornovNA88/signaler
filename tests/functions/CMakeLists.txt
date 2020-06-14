cmake_minimum_required(VERSION 3.8)

project(signaler-function-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GTEST_SOURCE_DIR "" CACHE PATH "Path to Google Test sources")
set(GTEST_WORKING_DIR "${CMAKE_BINARY_DIR}/googletest")

if (NOT EXISTS "${GTEST_SOURCE_DIR}")

set(GTEST_SOURCE_DIR "${GTEST_WORKING_DIR}/src" CACHE PATH "Path to Google Test sources" FORCE)
configure_file("${CMAKE_CURRENT_LIST_DIR}/CMakeLists.in"
               "${GTEST_WORKING_DIR}/tmp/CMakeLists.txt")
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
                WORKING_DIRECTORY "${GTEST_WORKING_DIR}/tmp")
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${GTEST_WORKING_DIR}/tmp")

endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include_directories(../../src/signal ../../src/function)

add_subdirectory("${GTEST_SOURCE_DIR}"
                 "${GTEST_WORKING_DIR}/build")
                 
add_executable(signaler-function-test "function_test.cpp")
target_link_libraries(signaler-function-test gtest_main)

add_test(signaler-function-test signaler-function-test)