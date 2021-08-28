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
    benchmark::ClobberMemory();
  }
}

void benchmark_global_function_string_view(benchmark::State& state) noexcept {
  for (auto _ : state) {
    global_function_string_view("call");
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_global_function)->UseRealTime();

BENCHMARK(benchmark_global_function_string_view)->UseRealTime();


void benchmark_emiting_signal_to_global_function(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect<global_function>();

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_emiting_signal_to_global_function)->UseRealTime();



struct Interface {
	virtual void virtual_function([[maybe_unused]] unsigned int& val) = 0;
};

struct Impl : Interface {

	virtual void virtual_function([[maybe_unused]] unsigned int& val) override {
    ++val;
  };

  void method([[maybe_unused]] unsigned int& val) {
    ++val;
  };

  void method_const_noexcept([[maybe_unused]] unsigned int& val) const noexcept {
    ++val;
  };
};

void benchmark_emiting_signal_to_virtual_method(benchmark::State& state) noexcept {

  Impl impl;
  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect<Impl,&Impl::virtual_function>(&impl);

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

void benchmark_emiting_signal_to_method(benchmark::State& state) noexcept {

  Impl impl;
  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect<Impl,&Impl::method>(&impl);

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

void benchmark_emiting_signal_to_method_const_noexcept(benchmark::State& state) noexcept {

  Impl impl;
  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect<Impl,&Impl::method_const_noexcept>(&impl);

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_emiting_signal_to_virtual_method)->UseRealTime();
BENCHMARK(benchmark_emiting_signal_to_method)->UseRealTime();
BENCHMARK(benchmark_emiting_signal_to_method_const_noexcept)->UseRealTime();


void benchmark_emiting_signal_to_lambda(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect([](unsigned int& val){ ++val; });

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_emiting_signal_to_lambda)->UseRealTime();

void benchmark_emiting_signal_to_lambda_with_context(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal;

  auto connection = signal.connect([state](unsigned int& val){ ++val; });

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_emiting_signal_to_lambda_with_context)->UseRealTime();

void benchmark_emiting_signal_to_3000000_lambdas(benchmark::State &state) noexcept {

  signal_t<void(unsigned int &)> signal;
  const size_t N = 3000000;
  std::vector<signal_t<void(unsigned int &)>::connection_t> connections(N);

  for (size_t i = 0; i < N; ++i)
    connections.emplace_back(signal.connect([](unsigned int &val) { ++val; }));

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_emiting_signal_to_3000000_lambdas)->UseRealTime();


void benchmark_emiting_signal_to_signal(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal,slot;

  auto connection_of_slot   = slot  .connect([](unsigned int& val){ ++val; });
  auto connection_of_signal = signal.connect(slot);

  for (auto _ : state) {
    signal(global_state);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_emiting_signal_to_signal)->UseRealTime();


void benchmark_connecting_signal_to_global_function(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal;
  signal_t<void(unsigned int&)>::connection_t connection;

  for (auto _ : state) {
    connection = signal.connect<global_function>();
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_connecting_signal_to_global_function)->UseRealTime();


void benchmark_connecting_signal_to_method(benchmark::State& state) noexcept {

  Impl impl;
  signal_t<void(unsigned int&)> signal;
  signal_t<void(unsigned int&)>::connection_t connection;

  for (auto _ : state) {
    connection = signal.connect<Impl,&Impl::method>(&impl);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_connecting_signal_to_method)->UseRealTime();


void benchmark_connecting_signal_to_lambda(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal;
  signal_t<void(unsigned int&)>::connection_t connection;

  for (auto _ : state) {
    connection = signal.connect([](unsigned int& val){ ++val; });
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_connecting_signal_to_lambda)->UseRealTime();

void benchmark_connecting_signal_to_lambda_with_context(benchmark::State& state) noexcept {

  signal_t<void(unsigned int&)> signal;
  signal_t<void(unsigned int&)>::connection_t connection;

  for (auto _ : state) {
    connection = signal.connect([state](unsigned int& val){ ++val; });
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_connecting_signal_to_lambda_with_context)->UseRealTime();


void benchmark_is_connected_signal_to_lambda_with_context(benchmark::State& state) noexcept {

  signal_t<void(unsigned int &)> signal;
  signal_t<void(unsigned int &)>::connection_t connection {

    signal.connect([state](unsigned int &val) { ++val; })
  };

  for (auto _ : state) {
    connection.is_connected();
    benchmark::ClobberMemory();
  }
}

BENCHMARK(benchmark_is_connected_signal_to_lambda_with_context)->UseRealTime();