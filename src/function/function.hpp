/**
  ******************************************************************************
  * @file             function.hpp
  * @brief            Simple delegate pattern
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

#ifndef _FUNCTION_
#define _FUNCTION_

#include <utility>
#include <type_traits>

#include "detail/storage.hpp"

namespace signaler {

	template < typename T > class function_t;

	template< typename R, typename ...A >
	class function_t< R(A...) > final {

		using wraper_t = R(*)(void*, A&&...);


		void*              functor = nullptr;
		wraper_t           aplly = nullptr;
		detail::storage_t  store;


		template < R(*f)(A...) >
		static R _aplly([[maybe_unused]] void* const, A&&... args) {

			return f(std::forward<A>(args)...);
		}

		template < typename T, R(T::*m)(A...) >
		static R _aplly(void* const o, A&&... args) {

			return (static_cast<T*>(o)->*m)(std::forward<A>(args)...);
		}

		template < typename T, R(T::*m)(A...) const >
		static R _aplly(void* const o, A&&... args) {

			return (static_cast<T const*>(o)->*m)(std::forward<A>(args)...);
		}

		template <typename T>
		static R _aplly(void* const o, A&&... args) {

			return (*static_cast<T*>(o))(std::forward<A>(args)...);
		}


		function_t(void* const o, wraper_t const m) :
			functor(o), aplly(m) {}

	public:

		function_t() = delete;

		function_t(std::nullptr_t const) {}

		template < typename T,
			typename = typename ::std::enable_if<
			!::std::is_same<function_t, typename ::std::decay<T>::type>::value >::type  >
			function_t(T&& f) {

			using functor_t = typename std::decay<T>::type;

			functor = store.init< T >(std::forward<T>(f));
			aplly = _aplly< functor_t >;
		}


		template < typename T >
		function_t(T* const o, R(T::* const m)(A...)) {

			*this = bind(o, m);
		}

		template < typename T >
		function_t(T* const o, R(T::* const m)(A...) const) {

			*this = bind(o, m);
		}

		template < typename T >
		function_t(T& o, R(T::* const m)(A...)) {

			*this = bind(o, m);
		}

		template < typename T >
		function_t(T const& o, R(T::* const m)(A...) const) {

			*this = bind(o, m);
		}


		function_t(function_t const& f) {

			functor = f.functor;
			aplly = f.aplly;
			store = f.store;
		}


		function_t(function_t&& f) {

			functor = f.functor;
			aplly = f.aplly;
			store = ::std::move(f.store);

			f.functor = nullptr;
			f.aplly = nullptr;
		}


		function_t& operator = (function_t const& f) {

			functor = f.functor;
			aplly = f.aplly;
			store = f.store;

			return *this;
		}


		function_t& operator = (function_t&& f) {

			functor = f.functor;
			aplly = f.aplly;
			store = ::std::move(f.store);

			f.functor = nullptr;
			f.aplly = nullptr;

			return *this;
		}


		template < typename T,
			typename = typename ::std::enable_if<
			!::std::is_same<function_t, typename ::std::decay<T>::type>::value  >::type >
			function_t& operator = (T&& f) {

			using functor_t = typename std::decay<T>::type;

			functor = store.init< T >(std::forward<T>(f));
			aplly = _aplly< functor_t >;

			return *this;
		}


		function_t& operator = (std::nullptr_t const null_object) {

			return *this = bind(null_object);
		}

		function_t& operator = (int const null_object) {

			return *this = bind(null_object);
		}


		template < R(*const f)(A...) >
		static function_t bind() {

			return { nullptr, _aplly<f> };
		}

		template < typename T, R(T::* const m)(A...) >
		static function_t bind(T* const o) {

			return { o, _aplly<T, m> };
		}

		template < typename T, R(T::* const m)(A...) const >
		static function_t bind(T const* const o) {

			return { const_cast<T*>(o), _aplly<T, m> };
		}

		template < typename T, R(T::* const m)(A...) >
		static function_t bind(T& o) {

			return { &o, _aplly<T, m> };
		}

		template < typename T, R(T::* const m)(A...) const >
		static function_t bind(T const& o) {

			return { const_cast<T*>(&o), _aplly<T, m> };
		}


		template < typename T >
		static function_t bind(T* const o, R(T::* const m)(A...)) {

			return [=](A&&... args) ->R { return (o->*m)(std::forward<A>(args)...); };
		}

		template < typename T >
		static function_t bind(T const* const o, R(T::* const m)(A...)const) {

			return [=](A&&... args) ->R { return (o->*m)(std::forward<A>(args)...); };
		}

		template < typename T >
		static function_t bind(T& o, R(T::* const m)(A...)) {

			return [=](A&&... args) ->R { return (o->*m)(std::forward<A>(args)...); };
		}

		template < typename T >
		static function_t bind(T const& o, R(T::* const m)(A...) const) {

			return [=](A&&... args) ->R { return (o->*m)(std::forward<A>(args)...); };
		}


		static function_t bind(std::nullptr_t const null_object) {

			return null_object;
		}

		static function_t bind(int const null_object) {

			return nullptr;
		}


		bool operator==(function_t const& r) const {

			return (functor == r.functor) && (aplly == r.aplly);
		}

		bool operator!=(function_t const& r) const {

			return !operator==(r);
		}

		bool operator<(function_t const& r) const {

			return (functor < r.functor) || ((functor == r.functor) && (aplly < r.aplly));
		}

		bool operator==(std::nullptr_t const) const {

			return !aplly;
		}

		bool operator!=(std::nullptr_t const) const {

			return aplly;
		}


		explicit operator bool() const {

			return aplly;
		}


		R operator()(A... args) const {

			return aplly(functor, std::forward<A>(args)...);
		}
	};

}

#endif  //_FUNCTION_
