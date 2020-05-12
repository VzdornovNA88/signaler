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

    template <typename T>
	class signal_t;


	template < typename R, typename ...A >
	class signal_t< R(A...) >  final {
	public:

		using slot_t = function_t<void(A...)>;

		class connection_t {

			template<typename T>
			class ret_value_t {
				T _val = {};
				void operator =(T _d) { _val = _d; }
			public:  operator T() { return _val; }
			};

			template<>
			class ret_value_t<void> {
			public:  operator void() { static_assert(false, "The return value is a VOID value"); return; }
			};

			slot_t         _slot;
			ret_value_t<R> _result;
		public:
			connection_t(const slot_t& _s) :_slot(_s) {}
			friend class signal_t< R(A...) >;

			R return_result() const { return _result; }
		};


		signal_t() { connections.reserve(8); }

		signal_t( signal_t&& other ) {

			connections.swap( other.connections );
			other.connections.clear();
		}

		signal_t& operator = ( signal_t&& other ) {

			disconnect();
			connections.swap(other.connections);
			other.connections.clear();
			return *this;
		}

		template <typename T, void (T::*m)(A...)>
		connection_t* connect( T* o ) {

			auto _reconnect_slot = function_t<void(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_reconnect_slot](auto _connection) {
				return _connection->_slot == _reconnect_slot;
			});

			if (it == connections.end()) {
				auto _c = new connection_t(function_t<void(A...)>::template bind<T, m>(o));
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;
		}

		template <typename T, void (T::*m)(A...)>
		void disconnect( T* o ) {

			auto _disconnect_slot = function_t<void(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](auto _connection) {
				return _connection->_slot == _disconnect_slot;
			});

			if (it != connections.end()) {
				auto _c = *it;
				connections.erase(it);
				delete _c;
			}
		}


		template <typename T, void (T::*m)(A...) const>
		connection_t* connect( T* o ) {

			auto _reconnect_slot = function_t<void(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_reconnect_slot](auto _connection) {
				return _connection->_slot == _reconnect_slot;
			});

			if (it == connections.end()) {
				auto _c = new connection_t(function_t<void(A...)>::template bind<T, m>(o));
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;
		}

		template <typename T, void (T::*m)(A...) const>
		void disconnect( T* o ) {

			auto _disconnect_slot = function_t<void(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](auto _connection) {
				return _connection->_slot == _disconnect_slot;
			});

			if (it != connections.end()) {
				auto _c = *it;
				connections.erase(it);
				delete _c;
			}
		}


		template <void(*f)(A...)>
		connection_t* connect() {

			auto _reconnect_slot = function_t<void(A...)>::template bind<f>();

			auto it = std::find_if(connections.begin(), connections.end(), [&_reconnect_slot](auto _connection) {
				return _connection->_slot == _reconnect_slot;
			});

			if (it == connections.end()) {
				auto _c = new connection_t(function_t<void(A...)>::template bind<f>());
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;
		}

		template <void(*f)(A...)>
		void disconnect() {

			auto _disconnect_slot = function_t<void(A...)>::template bind<f>();

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](auto _connection) {
				return _connection->_slot == _disconnect_slot;
			});

			if (it != connections.end()) {
				auto _c = *it;
				connections.erase(it);
				delete _c;
			}
		}


		connection_t* connect( slot_t const& slot ) {

			auto _c = new connection_t(slot);

			auto it = std::find_if(connections.begin(), connections.end(), [_c](auto _connection) {
				return _connection == _c;
			});

			if (it == connections.end()) {
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;			
		}


		connection_t* connect( signal_t& signal ) {
			return connect<signal_t,&signal_t::operator()>(&signal);
		}

		void disconnect(signal_t& _disconnect_signal) {

			auto _disconnect_slot = function_t<void(A...)>::template bind<signal_t, &signal_t::operator()>(&_disconnect_signal);

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](auto _connection) {
				return _connection->_slot == _disconnect_slot;
			});

			if (it != connections.end()) {
				auto _c = *it;
				connections.erase(it);
				delete _c;
			}
		}


		void disconnect( connection_t* _c ) {

			if (_c == nullptr) return;

			auto it = std::find_if(connections.begin(), connections.end(), [_c](auto _connection) {
				return _connection == _c;
			});

			if (it != connections.end()) {
				auto _c = *it;
				connections.erase(it);
				delete _c;
			}
		}


		void disconnect() {

			for (auto _c : connections)
				disconnect( _c );
		}


		template < typename = typename ::std::enable_if< !::std::is_same<R, void>::value >::type >
		void operator()(A... p) {

			for ( auto connection : connections ) {
				if( connection != nullptr )
					connection->_result = connection->_slot(std::forward<A>(p)...);
			}
		}

		template < typename = typename ::std::enable_if< ::std::is_same<R, void>::value >::type >
		void operator()(A... p) const {

			for (auto connection : connections) {
				if (connection != nullptr)
					connection->_slot(std::forward<A>(p)...);
			}
		}


		private: std::vector<connection_t*> connections;
	};

}

#endif  //_SIGNAL_