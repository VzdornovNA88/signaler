#include "benchmark/benchmark.h"
#include <signal.hpp>

using namespace signaler;

unsigned int global_state;

void global_function([[maybe_unused]] unsigned int& val)
{
  ++val;
}

void global_function_string_view([[maybe_unused]] std::string_view s)
{

}

void benchmark_global_function(benchmark::State& state) noexcept {
  for (auto _ : state) {
    global_function(global_state);
  }
}

void benchmark_global_function_string_view(benchmark::State& state) noexcept {
  for (auto _ : state) {
    global_function_string_view("call");
  }
}

BENCHMARK(benchmark_global_function);

BENCHMARK(benchmark_global_function_string_view);


void benchmark_emiting_signal_to_global_function(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect<global_function>();

  for (auto _ : state) {
    signal(global_state);
  }
}

BENCHMARK(benchmark_emiting_signal_to_global_function);



struct Interface {
	virtual void virtual_function([[maybe_unused]] unsigned int& val) = 0;
};

struct Impl : Interface {

	virtual void virtual_function([[maybe_unused]] unsigned int& val) override {
    ++val;
  };
};

void benchmark_emiting_signal_to_virtual_method(benchmark::State& state) noexcept {

  Impl impl;
  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect<Impl,&Impl::virtual_function>(&impl);

  for (auto _ : state) {
    signal(global_state);
  }
}

BENCHMARK(benchmark_emiting_signal_to_virtual_method);