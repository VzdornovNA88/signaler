/**
  ******************************************************************************
  * @file             signal.hpp
  * @brief            Simple signal-object pattern
  * @author           Nik A. Vzdornov (VzdornovNA88@yandex.ru)
  * @date             10.09.19
  * @copyright 
  *
  * Copyright (c) 2019 VzdornovNA88
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  *
  ****************************************************************************** 
  */

#ifndef _SIGNAL_
#define _SIGNAL_

#include "function.hpp"
#include <set>

namespace signals {

template <typename... Args>
class signal final {

  using slot_t = function<void(Args...)>;
  mutable std::set<slot_t> slots;

 public:

  using connection_id_t = typename std::set<slot_t>::const_iterator;

  signal() {}

  signal( signal const& other ) {
    
    slots = other.slots;
  }

  signal& operator = (signal const& other) {

    disconnect();
    slots = other.slots;
    return *this;
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

  void operator()( Args... p ) const {

    for(const auto& slot : slots)
      slot(std::forward<Args>(p)...);
  }
};

}

#endif  //_SIGNAL_