#include "function.hpp"
#include <set>

template <typename... Args>
class signal {

  using slot_t = function<void(Args...)>;

 public:

  signal() /*: current_id_(0)*/ {}

  signal(signal const& other) /*: current_id_(0) */{
    std::cout << "signal(signal const& other)" << std::endl;
    slots = other.slots;
  }

  template <typename T>
  void connect(T *inst, void (T::*func)(Args...)) {
    connect([=](Args... args) { 
      (inst->*func)(args...); 
    });
  }

  template <typename T>
  void connect(T *inst, void (T::*func)(Args...) const) {

    connect([=](Args... args) {
      (inst->*func)(args...); 
    });
  }

  void connect(slot_t const& slot) {

    slots.insert( new slot_t( slot ) );
    // return current_id_;
  }

  // void disconnect(int id) const {
  //   slots_.erase(id);
  // }

  void disconnect_all() const {
    slots.clear();
  }

  void operator()(Args... p) {
    std::cout << "void operator()(Args... p) start" << std::endl;
    for(auto const& slot : slots) {
      std::cout << "void operator()(Args... p)" << std::endl;
      (*slot)(std::forward<Args>(p)...);
    }
  }

  // signal& operator=(signal const& other) {
  //   disconnect_all();
  // }

 private:
  std::set<slot_t*> slots;
};