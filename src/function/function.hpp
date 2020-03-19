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

#include "detail/storage.hpp"
#include "detail/type_traits.hpp"

namespace signals {

template < typename T > class function_t;

template< typename R, typename ...A >
class function_t< R (A...) > final {

  using wraper_t = R (*)( void*, A&&... );


  void*              functor = nullptr;
  wraper_t           aplly   = nullptr;
  detail::storage_t  store   = nullptr;


  function_t( void* const o, wraper_t const m ) : 
  functor(o),aplly(m) {}


  template < typename T >
  using pair = std::pair< T* const, R (T::* const)(A...) >;
  template < typename T >
  using const_pair = std::pair< T const* const, R (T::* const)(A...) const >;

  template <typename>
  struct is_pair : std::false_type {};
  template <typename T>
  struct is_pair< std::pair< T* const,R (T::* const)(A...)> > : std::true_type {};

  template <typename>
  struct is_const_pair : std::false_type { };
  template <typename T>
  struct is_const_pair< std::pair<T const* const,R (T::* const)(A...) const> > : std::true_type {};


  template < R (*f)(A...) >
  static R _aplly( void* const, A&&... args ) {

    return f( std::forward<A>(args)... );
  }

  template < typename T, R (T::*m)(A...) >
  static R _aplly( void* const o, A&&... args ) {

    return (static_cast<T*>(o)->*m)( std::forward<A>(args)... );
  }

  template < typename T, R (T::*m)(A...) const >
  static R _aplly(void* const o, A&&... args) {

    return (static_cast<T const*>(o)->*m)( std::forward<A>(args)... );
  }

  template <typename T>
  static typename std::enable_if< !(is_pair<T>::value || 
                                    is_const_pair<T>::value), R >::
  type _aplly(void* const o, A&&... args) {

    return (*static_cast<T*>(o))(std::forward<A>(args)...);
  }

  template <typename T>
  static typename std::enable_if< is_pair<T>::value ||                  
                                  is_const_pair<T>::value, R >::
  type _aplly(void* const _o, A&&... args) {

    auto o = static_cast<T*>(_o)->first;
    auto m = static_cast<T*>(_o)->second;

    return (o->*m)( std::forward<A>(args)... );
  }

  static R _aplly(void* const o, A&&... args) {

    return (*reinterpret_cast<R(*)(A...)>(o))(std::forward<A>(args)...);
  }

public:

  function_t() = default;

  function_t( std::nullptr_t const ) : function_t() {}

  template < typename T >
  function_t( T* const o, R (T::* const m)(A...) ) {

    *this = bind( o, m );
  }

  template < typename T >
  function_t( T* const o, R (T::* const m)(A...) const ) {

    *this = bind( o, m );
  }

  template < typename T >
  function_t( T& o, R (T::* const m)(A...) ) {

    *this = bind( o, m );
  }

  template < typename T >
  function_t( T const& o, R (T::* const m)(A...) const ) {

    *this = bind( o, m );
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
             !::std::is_function<T>::value                                     &&
             !( ::std::is_empty<T>::value                                      && 
                detail::is_functor<T,R(A...)>::value ) >::type  >
  function_t(T&& f) {

    using functor_t = typename std::decay<T>::type;

    functor = store.make< T >( std::forward<T>(f) );
    aplly   = _aplly< functor_t >;
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
             ( ::std::is_function<T>::value                                    ||
               ( ::std::is_empty<T>::value                                     && 
                 detail::is_functor<T,R(A...)>::value ) ) >::type  >
  function_t(T f) {

    functor = reinterpret_cast< void* >( static_cast< R(*)(A...) >(f) );
    aplly   = _aplly;
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
             ( ::std::is_function<T>::value                                    ||
               ( ::std::is_empty<T>::value                                     && 
                 detail::is_functor<T,R(A...)>::value ) ) >::type  >
  function_t(T* f) {

    functor = reinterpret_cast< void* >( static_cast< R(*)(A...) >(f) );
    aplly   = _aplly;
  }


function_t( function_t const& f ) {

    functor = f.functor;
    aplly   = f.aplly;
    store   = f.store;
  }


function_t( function_t&& f ) {

    functor = f.functor;
    aplly   = f.aplly;
    store   = f.store;

    f.functor = nullptr;
    f.aplly   = nullptr;
    f.store   = nullptr;
  }


  function_t& operator = ( function_t const& f ) {

    functor = f.functor;
    aplly   = f.aplly;
    store   = f.store;

    return *this;
  }


  function_t& operator = ( function_t&& f ) {

    functor = f.functor;
    aplly   = f.aplly;
    store   = f.store;

    f.functor = nullptr;
    f.aplly   = nullptr;
    f.store   = nullptr;

    return *this;
  }


  template < typename T >
  function_t& operator = ( R (T::* const m)(A...) ) {

    return *this = bind( static_cast<T*>(functor), m );
  }

  template < typename T >
  function_t& operator=( R (T::* const m)(A...) const ) {

    return *this = bind( static_cast<T const*>(functor), m );
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
             !::std::is_function<T>::value                                     &&
             !( ::std::is_empty<T>::value                                      && 
                detail::is_functor<T,R(A...)>::value ) >::type  >
  function_t& operator = ( T&& f ) {

    using functor_t = typename std::decay<T>::type;

    functor = store.make< T >( std::forward<T>(f) );
    aplly   = _aplly< functor_t >;

    return *this;
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
             ( ::std::is_function<T>::value                                    ||
               ( ::std::is_empty<T>::value                                     && 
                 detail::is_functor<T,R(A...)>::value ) ) >::type  >
  function_t& operator = ( T f ) {

    functor = reinterpret_cast< void* >( static_cast< R(*)(A...) >(f) );
    aplly   = _aplly;
    store   = nullptr;

    return *this;
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
             ( ::std::is_function<T>::value                                    ||
               ( ::std::is_empty<T>::value                                     && 
                 detail::is_functor<T,R(A...)>::value ) ) >::type  >
  function_t& operator = ( T* f ) {

    functor = reinterpret_cast< void* >( static_cast< R(*)(A...) >(f) );
    aplly   = _aplly;
    store   = nullptr;

    return *this;
  }

  function_t& operator = ( std::nullptr_t const null_object ) {

    return *this = bind( null_object );
  }

  function_t& operator = ( int const null_object ) {

    return *this = bind( null_object );
  }


  template < R (* const f)(A...) >
  static function_t bind() {

    return { nullptr, _aplly<f> };
  }

  template < typename T, R (T::* const m)(A...) >
  static function_t bind( T* const o ) {

    return { o, _aplly<T, m> };
  }

  template < typename T, R (T::* const m)(A...) const >
  static function_t bind( T const* const o ) {

    return { const_cast<T*>(o), _aplly<T, m> };
  }

  template < typename T, R (T::* const m)(A...) >
  static function_t bind( T& o ) {

    return { &o, _aplly<T, m> };
  }

  template < typename T, R (T::* const m)(A...) const >
  static function_t bind( T const& o ) {

    return { const_cast<T*>(&o), _aplly<T, m> };
  }

  template < typename T >
  static function_t bind( T&& f ) {

    return std::forward<T>( f );
  }

  static function_t bind( R (* const f)(A...) )  {

    return f;
  }

  template < typename T >
  static function_t bind( T* const o, R (T::* const m)(A...) ) {

    return pair<T>( o, m );
  }

  template < typename T >
  static function_t bind( T const* const o, R (T::* const m)(A...)const ) {

    return const_pair<T>( o, m );
  }

  template < typename T >
  static function_t bind( T& o, R (T::* const m)(A...) ) {

    return pair<T>( &o, m );
  }

  template < typename T >
  static function_t bind( T const& o, R (T::* const m)(A...) const ) {

    return const_pair<T>( &o, m );
  }

  static function_t bind( std::nullptr_t const null_object ) {

    return null_object;
  }

  static function_t bind( int const null_object ) {

    return nullptr;
  }


  void swap( function_t& other ) { 

    std::swap( *this, other ); 
  }


  bool operator==( function_t const& r ) const {

    return (functor == r.functor) && (aplly == r.aplly);
  }

  bool operator!=( function_t const& r ) const {

    return !operator==(r);
  }

  bool operator<( function_t const& r ) const {

    return (functor < r.functor) || ((functor == r.functor) && (aplly < r.aplly));
  }

  bool operator==( std::nullptr_t const ) const {

    return !aplly;
  }

  bool operator!=( std::nullptr_t const ) const {

    return aplly;
  }


  explicit operator bool() const { 

    return aplly; 
  }


  R operator()( A... args ) const {

    return aplly( functor, std::forward<A>(args)... );
  }
};

}

#endif  //_FUNCTION_
