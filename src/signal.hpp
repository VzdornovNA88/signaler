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

#include "function/function.hpp"
// #include <set>
#include <vector>
// #include <unordered_set>
#include <algorithm>


namespace signaler {

// template <typename... A>
// class signal_t final {

// 	struct FunctionHasher
// 	{
// 		size_t
// 			operator()(const function_t<void(A...)> & obj) const
// 		{
// 			return std::hash<size_t>()(obj.hash_code());
// 		}
// 	};

// 	// Custom comparator that compares the string objects by length
// 	struct FunctionComparator
// 	{
// 		bool
// 			operator()(const function_t<void(A...)> & obj1, const function_t<void(A...)> & obj2) const
// 		{
// 			if (obj1 == obj2)
// 				return true;
// 			return false;
// 		}
// 	};

//  using slot_t = function_t<void(A...)>;
//  mutable std::unordered_set<slot_t, FunctionHasher, FunctionComparator> slots;

// public:

//  signal_t() {}

//  signal_t( signal_t const& other ) {
   
//    slots = other.slots;
//  }

//  signal_t& operator = (signal_t const& other) {

//    disconnect();
//    slots = other.slots;
//    return *this;
//  }
 
//  template <typename T>
//  void connect( T *o, void (T::*m)(A...) ) const {

//    slots.emplace( o,m );
//  }

//  template <typename T>
//  	void disconnect(T* o, void (T::*m)(A...)) const {

//  		slots.erase(function_t<void(A...)>(o, m));
//  	}


//  template <typename T>
//  void connect( T *o, void (T::*m)(A...) const ) const {

//    slots.emplace( o,m );
//  }

//  template <typename T>
//  void disconnect(T* o, void (T::*m)(A...) const ) const {

// 	slots.erase(function_t<void(A...)>(o, m));
//  }


//  template <typename T, void (T::*m)(A...)>
//  void connect( T *o ) const {

//    slots.insert( function_t<void (A...)>::template bind<T, m>( o ) );
//  }

//  template <typename T, void (T::*m)(A...)>
//  void disconnect(T* o) const {

// 	slots.erase(function_t<void(A...)>::template bind<T, m>(o));
//  }


//  template <typename T, void (T::*m)(A...) const>
//  void connect( T *o ) const {

//    slots.insert( function_t<void (A...)>::template bind<T, m>( o ) );
//  }

//  template <typename T, void (T::*m)(A...) const>
//  void disconnect(T* o) const {

// 	slots.erase(function_t<void(A...)>::template bind<T, m>(o));
//  }


//  void connect( slot_t const& slot ) const {

//    slots.emplace( slot );
//  }

//  	void disconnect(slot_t const& _disconnect_slot) const {

// 			slots.erase(_disconnect_slot);
// 		}


// 		void connect(signal_t& _signal) const {
// 			connect(&_signal, &signal_t::operator());
// 		}

// 		void disconnect(signal_t& _disconnect_slot) const {

// 			disconnect(&_disconnect_slot, &signal_t::operator());
// 		}


//  void disconnect() const {

//    slots.clear();
//  }

//  void operator()( A... p ) const {

//    for(const auto& slot : slots)
//      slot(std::forward<A>(p)...);
//  }
// };


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////



	template <typename... A>
	class signal_t final {

		using slot_t = function_t<void(A...)>;
		mutable std::vector<slot_t> slots;

	public:

		signal_t() { slots.reserve(8); }

		signal_t(signal_t const& other) {

			slots = other.slots;
		}

		signal_t& operator = (signal_t const& other) {

			disconnect();
			slots = other.slots;
			return *this;
		}

		template <typename T>
		void connect(T* o, void (T::*m)(A...)) const {

			return slots.emplace_back(o, m);
		}

		template <typename T>
		void disconnect(T* o, void (T::*m)(A...)) const {

			function_t<void(A...)> _disconnect_slot(o, m);
			
			slots.erase( std::find_if(slots.begin(), slots.end(), [&_disconnect_slot](auto _slot) {
				return _slot == _disconnect_slot;
			}));
		}


		template <typename T>
		void connect(T* o, void (T::*m)(A...) const) const {

			return slots.emplace_back(o, m);
		}

		template <typename T>
		void disconnect(T* o, void (T::*m)(A...) const) const {

			function_t<void(A...)> _disconnect_slot(o, m);

			slots.erase(std::find_if(slots.begin(), slots.end(), [&_disconnect_slot](auto _slot) {
				return _slot == _disconnect_slot;
			}));
		}


		template <typename T, void (T::*m)(A...)>
		void connect(T* o) const {

			return slots.push_back(function_t<void(A...)>::template bind<T, m>(o));
		}

		template <typename T, void (T::*m)(A...)>
		void disconnect(T* o) const {

			auto _disconnect_slot = function_t<void(A...)>::template bind<T, m>(o);

			slots.erase(std::find_if(slots.begin(), slots.end(), [&_disconnect_slot](auto _slot) {
				return _slot == _disconnect_slot;
			}));
		}


		template <typename T, void (T::*m)(A...) const>
		void connect(T* o) const {

			return slots.push_back(function_t<void(A...)>::template bind<T, m>(o));
		}

		template <typename T, void (T::*m)(A...) const>
		void disconnect(T* o) const {

			auto _disconnect_slot = function_t<void(A...)>::template bind<T, m>(o);

			slots.erase(std::find_if(slots.begin(), slots.end(), [&_disconnect_slot](auto _slot) {
				return _slot == _disconnect_slot;
			}));
		}


		void connect(slot_t const& slot) const {
			return slots.emplace_back(slot);
		}

		void disconnect(slot_t const& _disconnect_slot) const {

			slots.erase(std::find_if(slots.begin(), slots.end(), [&_disconnect_slot](auto _slot) {
				return _slot == _disconnect_slot;
			}));
		}


		void connect(signal_t& signal) const {
			return connect(&signal, &signal_t::operator());
		}

		void disconnect(signal_t& _disconnect_slot) const {

			slots.erase(std::find_if(slots.begin(), slots.end(), [&_disconnect_slot](auto _slot) {
				return _slot == _disconnect_slot;
			}));
		}


		void disconnect() const {

			slots.clear();
		}

		void operator()(A... p) const {

			for (const auto& slot : slots) {
				slot(std::forward<A>(p)...);
			}
		}
	};

}

#endif  //_SIGNAL_