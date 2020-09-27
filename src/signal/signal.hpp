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

#ifndef SIGNALER_SIGNAL
#define SIGNALER_SIGNAL

#include "../function/function.hpp"
#include "detail/id_connection_generator.hpp"

#include <vector>
#include <algorithm>
#include <memory>


namespace signaler {

	template <typename T>
	class signal_t;

	template < typename R, typename ...A >
	class signal_t< R(A...) >  final {
	public:
		using slot_t = function_t<R(A...)>;

	private:
		class connection_base_t {
			slot_t _slot  = nullptr;
			long long _id = id_t::UNDEFINED;

			connection_base_t(slot_t&& _s) :_slot(std::forward<slot_t>(_s)), _id(id_t::get()) {}
			connection_base_t(const slot_t& _s) :_slot(_s), _id(id_t::get()) {}
		public:
			connection_base_t() = default;

			friend class signal_t< R(A...) >;

			bool operator<(connection_base_t const& r) const {

				return (_id < r._id);
			}

			bool operator==(connection_base_t const& r) const {

				return (_id == r._id);
			}
		};

		class connection_base_with_result_t : public connection_base_t {
			template<typename T>
			class ret_value_t {

				friend class signal_t< R(A...) >;

				T _val = {};
				void operator =(T _d) { _val = _d; }

			public:
				operator T() const { return _val; }
			};

			connection_base_with_result_t(slot_t&& _s) :connection_base_t(std::forward<slot_t>(_s)),
				_result(std::make_shared<ret_value_t<R>>()) {}

			connection_base_with_result_t(const slot_t& _s) :connection_base_t(_s),
				_result(std::make_shared<ret_value_t<R>>()) {}

			mutable std::shared_ptr<ret_value_t<R>> _result;
		public:

			connection_base_with_result_t() = default;

			friend class signal_t< R(A...) >;

			[[nodiscard]] R signal_result() const { return *_result; }
		};

	public:
		using connection_t = typename std::conditional<
			std::is_same_v<R, void>, connection_base_t, connection_base_with_result_t>::type;


		signal_t() = default;

		signal_t(signal_t&& other) {

			connections.swap(other.connections);
			other.connections.clear();
		}

		~signal_t() {
			disconnect();
		}

		signal_t& operator = (signal_t&& other) {

			disconnect();
			connections.swap(other.connections);
			other.connections.clear();
			return *this;
		}


		template <typename T, R(T::*m)(A...)>
		connection_t connect(T* o) {

			auto _slot = function_t<R(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_slot](const auto& _connection) {
				return _connection._slot == _slot;
			});

			if (it == connections.end()) {
				auto _c = connection_t(std::forward<slot_t>(_slot));
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;
		}

		template <typename T, R(T::*m)(A...)>
		void disconnect(T* o) {

			auto _disconnect_slot = function_t<R(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](const auto& _connection) {
				return _connection._slot == _disconnect_slot;
			});

			if (it != connections.end())
				connections.erase(it);
		}


		template <typename T, R(T::*m)(A...) const>
		connection_t connect(T* o) {

			auto _slot = function_t<R(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_slot](const auto& _connection) {
				return _connection._slot == _slot;
			});

			if (it == connections.end()) {
				auto _c = connection_t(std::forward<slot_t>(_slot));
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;
		}

		template <typename T, R(T::*m)(A...) const>
		void disconnect(T* o) {

			auto _disconnect_slot = function_t<R(A...)>::template bind<T, m>(o);

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](const auto& _connection) {
				return _connection._slot == _disconnect_slot;
			});

			if (it != connections.end())
				connections.erase(it);
		}


		template <R(*f)(A...)>
		connection_t connect() {

			auto _slot = function_t<R(A...)>::template bind<f>();

			auto it = std::find_if(connections.begin(), connections.end(), [&_slot](const auto& _connection) {
				return _connection._slot == _slot;
			});

			if (it == connections.end()) {
				auto _c = connection_t(std::forward<slot_t>(_slot));
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;
		}

		template <R(*f)(A...)>
		void disconnect() {

			auto _disconnect_slot = function_t<R(A...)>::template bind<f>();

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](const auto& _connection) {
				return _connection._slot == _disconnect_slot;
			});

			if (it != connections.end())
				connections.erase(it);
		}


		[[nodiscard]] connection_t connect(slot_t&& slot) {

			auto it = std::find_if(connections.begin(), connections.end(), [&slot](const auto& _connection) {
				return _connection._slot == slot;
			});

			if (it == connections.end()) {
				auto _c = connection_t(std::forward<slot_t>(slot));
				connections.push_back(_c);
				return _c;
			}
			else
				return *it;
		}


		connection_t connect(signal_t& signal) {
			return connect<signal_t, &signal_t::operator()>(&signal);
		}

		void disconnect(signal_t& _disconnect_signal) {

			auto _disconnect_slot =
				function_t<R(A...)>::template bind<signal_t, &signal_t::operator()>(&_disconnect_signal);

			auto it = std::find_if(connections.begin(), connections.end(), [&_disconnect_slot](const auto& _connection) {
				return _connection._slot == _disconnect_slot;
			});

			if (it != connections.end())
				connections.erase(it);
		}

	    
		void disconnect(const connection_t& _c) {

			auto it = std::lower_bound(connections.begin(), connections.end(), _c, std::less<>{});

			if (it != connections.end())
				connections.erase(it);
		}


		void disconnect() {

			for (const auto& _c : connections)
				disconnect(_c);
		}


		constexpr void operator()(A... p) const {

			for (const auto& connection : connections) {
				if constexpr (std::is_same<R, void>::value)
					connection._slot(std::forward<A>(p)...);
				else
					*connection._result =
					connection._slot(std::forward<A>(p)...);
			}
		}

		// что делать с протухшими копиями конекшенов(копии конекшенов при вызове коннекта) , которые пользователь хранит у себя в коде?
		//
	private: std::vector<connection_t> connections;
	};

}

#endif  // SIGNALER_SIGNAL