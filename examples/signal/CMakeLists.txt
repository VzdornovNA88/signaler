cmake_minimum_required (VERSION 3.8)

IF (WIN32)
  # set stuff for windows
ELSE()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread ")
ENDIF()

set(SOURCES "../../src/context/detail/inter_contexts_queue/queue_status.cpp" "../../src/signal/signal.cpp" "../../src/function/function.cpp" "signal_examples.cpp")

include_directories(../../src/signal ../../src/function ../../src/context ../../src/detail ../../src/platforms ../../src/result)

add_executable (signal-examples ${SOURCES})
