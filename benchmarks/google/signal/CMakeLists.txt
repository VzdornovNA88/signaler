cmake_minimum_required (VERSION 3.8)

IF (WIN32)
  # set stuff for windows
ELSE()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread ")
ENDIF()

set(
  SOURCES 
"../../../src/context/detail/inter_contexts_queue/queue_status.cpp" 
"../../../src/signal/signal.cpp" 
"../../../src/function/function.cpp" 
)

include_directories(
  ../../../src/signal
  ../../../src/function 
  ../../../src/context 
  ../../../src/detail 
  ../../../src/result
  )


macro(add_benchmark benchmark_name file)
  add_executable(signaler_google_benchmark_${benchmark_name} ${SOURCES} "${file}")
  target_link_libraries(signaler_google_benchmark_${benchmark_name} benchmark benchmark_main)
endmacro(add_benchmark benchmark_name)

set(BENCHMARK_ENABLE_TESTING OFF)

if(NOT EXISTS benchmark)
  execute_process(
    COMMAND git clone https://github.com/google/benchmark.git 
   WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()

add_subdirectory(benchmark)

add_benchmark(signal signal.cpp)