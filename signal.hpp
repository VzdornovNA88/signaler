#ifndef _SIGNAL_
#define _SIGNAL_

#include "function.hpp"
#include <set>

namespace signals {

template <typename... Args>
class signal {

  using slot_t          = function<void(Args...)>;
  
 public:

  using connection_id_t = typename std::set<slot_t>::const_iterator;

  signal() {}

  signal( signal const& other ) {
    std::cout << "signal(signal const& other)" << std::endl;
    slots = other.slots;
  }

  template <typename T>
  auto connect( T *inst, void (T::*func)(Args...) ) const {
    return slots.emplace( inst,func ).first;
  }

  template <typename T>
  auto connect( T *inst, void (T::*func)(Args...) const ) const {
    return slots.emplace( inst,func ).first;
  }

  connection_id_t connect( slot_t const& slot ) const {
    return slots.emplace( slot ).first;
  }

  void disconnect( connection_id_t id ) const {
    slots.erase( id );
  }

  void disconnect() const {
    slots.clear();
  }

  void operator()( Args... p ) {
    std::cout << "void operator()(Args... p) start" << std::endl;
    for(const auto& slot : slots) {
      std::cout << "void operator()(Args... p)" << std::endl;
      slot(std::forward<Args>(p)...);
    }
  }

  // signal& operator=(signal const& other) {
  //   disconnect_all();
  // }

 private:
  mutable std::set<slot_t> slots;
};

}

#endif  //_SIGNAL_