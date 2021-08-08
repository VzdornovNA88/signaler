#include <iostream>
#include <signal.hpp>
#include <thread>

using namespace signaler;

signaler::context_t<> context_ping;
using object_ping_t = signaler::object_t<&context_ping>;

struct ping_t : object_ping_t {

  void operator()(int &i) {
    std::cout << "{ PING 'void ping_t::foo(int i)' } : ++i = " << ++i
              << " in thread id: " << std::this_thread::get_id();

    std::cout << std::endl;
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if(!connection.is_connected())
      std::cout << "ping is disconnected" << std::endl;

    signal(i);
  }

  signal_t<void(int &)> signal;
  signal_t<void(int &)>::connection_t connection;
};

signaler::context_t<> context_pong;
using object_pong_t = signaler::object_t<&context_pong>;

struct pong_t : object_pong_t {

  void operator()(int &i) {
    std::cout << "{ PONG 'void pong_t::foo(int i)' } : ++i = " << ++i
              << " in thread id: " << std::this_thread::get_id();

    std::cout << std::endl;
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if(!connection.is_connected())
      std::cout << "pong is disconnected" << std::endl;

    signal(i);
  }

  signal_t<void(int &)> signal;
  signal_t<void(int &)>::connection_t connection;
};

int ping_pong_state = 0;

int main([[maybe_unused]]int argc, [[maybe_unused]]char *argv[]) {
  std::cout << "Begine example 'ping pong with signal/slot connections' : "
            << std::endl;

  std::cout << std::endl;
  std::cout << std::endl;

  std::thread ping_thread([] { context_ping(); });
  std::thread pong_thread([] { context_pong(); });

  std::cout << "Main thread ID : " << std::this_thread::get_id() << std::endl;
  std::cout << "Ping thread ID : " << ping_thread.get_id() << std::endl;
  std::cout << "Pong thread ID : " << pong_thread.get_id() << std::endl;

  ping_t ping;
  pong_t pong;

  ping.connection = ping.signal.connect(&pong);
  pong.connection = pong.signal.connect(&ping);

  signal_t<void(int &)> start;

  auto connection_ping_pong = start.connect(&ping);

  start(ping_pong_state);

  ping_thread.join();
  pong_thread.join();

  return 0;
}