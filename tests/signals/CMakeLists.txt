cmake_minimum_required(VERSION 3.8)

project(signaler-signals-test)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include_directories(../../src/signal ../../src/function)

#add_subdirectory("${GTEST_SOURCE_DIR}"
#                 "${GTEST_WORKING_DIR}/build")
                 
add_executable(signaler-signals-test "signal_test.cpp")
target_link_libraries(signaler-signals-test gtest_main)

add_test(signaler-signals-test signaler-signals-test)