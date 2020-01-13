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

#include <cmath>

namespace signals {

template < typename T > class function;

template< class R, class ...A >
class function< R (A...) >
{
  using wraper_t  = R (*)( void*, A&&... );
  using deleter_t = void (*)( void* );

  /// Function content
  void*           object             = nullptr;
  wraper_t        wraper             = nullptr;
  void*           store              = nullptr;
  deleter_t       storage_deleter    = nullptr;
  unsigned short* ref_store_counter  = nullptr;

  /**
    ****************************************************************************** 
    * @todo instead of three pointers i need to use 
    * structure of storage for functors, it is main 
    * idea for optimisation of memory
    *
    * template <typename T> struct store_t {
    *    using deleter_t = void (*)( void* );   
    *    void*           store              = nullptr;
    *    deleter_t       storage_deleter    = nullptr;
    *    unsigned short* ref_store_counter  = nullptr;
    * } 
    */

  function( void* const o, wraper_t const m ) noexcept : 
  object(o),wraper(m) {}

  /// Private type traits
  template < class T >
  using pair = std::pair< T* const, R (T::* const)(A...) >;

  template < class T >
  using const_pair = std::pair< T const* const, R (T::* const)(A...) const >;

  template <typename>
  struct is_pair : std::false_type {};
  template <class T>
  struct is_pair< std::pair< T* const,R (T::* const)(A...)> > : std::true_type {};

  template <typename>
  struct is_const_pair : std::false_type { };
  template <class T>
  struct is_const_pair< std::pair<T const* const,R (T::* const)(A...) const> > : std::true_type {};

  /// Wrapers
  template < R (*f)(A...) >
  static R f_wraper( void* const, A&&... args ) {
    
    std::cout << "f_wraper" << std::endl;
    return f( std::forward<A>(args)... );
  }

  template < class T, R (T::*m)(A...) >
  static R m_wraper( void* const o, A&&... args ) {

    std::cout << "m_wraper" << std::endl;
    return (static_cast<T*>(o)->*m)( std::forward<A>(args)... );
  }

  template < class T, R (T::*m)(A...) const >
  static R m_wraper_const(void* const o, A&&... args) {

    std::cout << "m_wraper_const" << std::endl;
    return (static_cast<T const*>(o)->*m)( std::forward<A>(args)... );
  }

  template <typename T>
  static typename std::enable_if< !(is_pair<T>::value || 
                                      is_const_pair<T>::value), R >::type
  call_operator(void* const o, A&&... args) {

    std::cout << "call_operator 1" << std::endl;
    return (*static_cast<T*>(o))(std::forward<A>(args)...);
  }

  template <typename T>
  static typename std::enable_if< is_pair<T>::value ||                  
                                    is_const_pair<T>::value, R >::type
  call_operator(void* const _o, A&&... args) {

    std::cout << "call_operator 2" << std::endl;
    auto o = static_cast<T*>(_o)->first;
    auto m = static_cast<T*>(_o)->second;
    return (o->*m)( std::forward<A>(args)... );
  }

  template < class T >
  static void deleter( void* _store ) {

    static_cast< T* >(_store)->~T();
    operator delete (_store);
    _store = nullptr;
  }
  
  void destructor() {

    if( store             != nullptr && 
        ref_store_counter != nullptr && *ref_store_counter == 0 ) {

      storage_deleter( store );
      delete ref_store_counter;
      ref_store_counter = nullptr;

      std::cout << "destruct" << std::endl;
    }
    else
    if( ref_store_counter != nullptr && *ref_store_counter > 0 ) {

      --(*ref_store_counter);
      store = nullptr;

      std::cout << "decrement" << std::endl;
    }
  }

public:

  ~function() { destructor(); }

  function() = default;

  function( function const& f ) {

    if( store != nullptr ) 
      destructor();

    object            = f.object;
    wraper            = f.wraper;
    store             = f.store;
    storage_deleter   = f.storage_deleter;
    ref_store_counter = f.ref_store_counter;

    if( ref_store_counter != nullptr )
      ++(*ref_store_counter);

    std::cout << "function( function const& f ) " <<std::endl;
  }

  function( function&& f ) {
    
    if( store != nullptr ) 
      destructor();

    object            = f.object;
    wraper            = f.wraper;
    store             = f.store;
    storage_deleter   = f.storage_deleter;
    ref_store_counter = f.ref_store_counter;

    if( ref_store_counter != nullptr )
      ++(*ref_store_counter);

    std::cout << "function( function&& f ) " << std::endl;
  }

  function( std::nullptr_t const ) noexcept : function() {}

  template < class T, 
  typename = typename std::enable_if< std::is_class<T>::value >::type >
  explicit function( T const* const o ) noexcept : object( const_cast<T*>(o) ) {}

  template < class T, 
  typename = typename std::enable_if< std::is_class<T>::value >::type >
  explicit function( T const& o ) noexcept : object( const_cast<T*>(&o) ) {}

  template < class T >
  function( T* const o, R (T::* const m)(A...) ) {
    *this = bind( o, m );
  }

  template < class T >
  function( T* const o, R (T::* const m)(A...) const ) {

    *this = bind( o, m );
  }

  template < class T >
  function( T& o, R (T::* const m)(A...) ) {

    *this = bind( o, m );
  }

  template < class T >
  function( T const& o, R (T::* const m)(A...) const ) {

    *this = bind( o, m );
  }

  template < typename T,
  typename = typename ::std::enable_if< 
     !::std::is_same<function, typename ::std::decay<T>::type>::value >::type >
  function(T&& f) : 
  store( operator new( sizeof(typename std::decay<T>::type) ) ),
  ref_store_counter( new unsigned short(0) ) {

    std::cout << "function(T&& f) - start " << std::endl;

    using functor_t = typename std::decay<T>::type;

    new (store) functor_t( std::forward<T>(f) );

    storage_deleter = deleter< functor_t >;
    object          = store;
    wraper          = call_operator< functor_t >;
  }

  function& operator=( function const& f ) {

    std::cout << "function& operator=( function const& f )start " << std::endl;

    if( store != nullptr ) 
      destructor();

    object            = f.object;
    wraper            = f.wraper;
    store             = f.store;
    storage_deleter   = f.storage_deleter;
    ref_store_counter = f.ref_store_counter;

    if( ref_store_counter != nullptr )
      ++(*ref_store_counter);

    std::cout << "function& operator=( function const& f ) " << std::endl;

    return *this;
  }

  function& operator=( function&& f ) {
    
    if( store != nullptr ) 
      destructor();

    object            = f.object;
    wraper            = f.wraper;
    store             = f.store;
    storage_deleter   = f.storage_deleter;
    ref_store_counter = f.ref_store_counter;

    if( ref_store_counter != nullptr )
      ++(*ref_store_counter);

    std::cout << "function& operator=( function&& f ) " << std::endl;

    return *this;
  }

  template < class T >
  function& operator=( R (T::* const m)(A...) ) {

    return *this = bind( static_cast<T*>(object), m );
  }

  template < class T >
  function& operator=( R (T::* const m)(A...) const ) {

    return *this = bind( static_cast<T const*>(object), m );
  }

  template < typename T,
  typename = typename ::std::enable_if< 
     !::std::is_same<function, typename ::std::decay<T>::type>{} >::type  >
  function& operator=( T&& f ) {

    std::cout << "function& operator=( T&& f ) - start " << std::endl;
    using functor_t = typename std::decay<T>::type;

    if( store != nullptr ) 
      destructor();

    store = operator new( sizeof(functor_t) );
    ref_store_counter = new unsigned short(0);

    new (store) functor_t( std::forward<T>(f) );

    storage_deleter = deleter< functor_t >;
    object          = store;
    wraper          = call_operator< functor_t >;

    return *this;
  }

  function& operator=( std::nullptr_t const null_object ) {

    if( store != nullptr ) 
      destructor();

    return *this = bind( null_object );
  }

  function& operator=( int const null_object ) {

    if( store != nullptr ) 
      destructor();

    return *this = bind( null_object );
  }

  template < R (* const f)(A...) >
  static function bind() noexcept {

    return { nullptr, f_wraper<f> };
  }

  template < class T, R (T::* const m)(A...) >
  static function bind( T* const o ) noexcept {

    return { o, m_wraper<T, m> };
  }

  template < class T, R (T::* const m)(A...) const >
  static function bind( T const* const o ) noexcept {

    return { const_cast<T*>(o), m_wraper_const<T, m> };
  }

  template < class T, R (T::* const m)(A...) >
  static function bind( T& o ) noexcept {

    return { &o, m_wraper<T, m> };
  }

  template < class T, R (T::* const m)(A...) const >
  static function bind( T const& o ) noexcept {

    return { const_cast<T*>(&o), m_wraper_const<T, m> };
  }

  template < typename T >
  static function bind( T&& f ) {

    return std::forward<T>( f );
  }

  static function bind( R (* const f)(A...) )  {

    return f;
  }

  template < class T >
  static function bind( T* const o, R (T::* const m)(A...) ) {

    return pair<T>( o, m );
  }

  template < class T >
  static function bind( T const* const o, R (T::* const m)(A...)const ) {

    return const_pair<T>( o, m );
  }

  template < class T >
  static function bind( T& o, R (T::* const m)(A...) ) {

    return pair<T>( &o, m );
  }

  template < class T >
  static function bind( T const& o, R (T::* const m)(A...) const ) {

    return const_pair<T>( &o, m );
  }

  static function bind( std::nullptr_t const null_object ) {

    return null_object;
  }

  static function bind( int const null_object ) {

    return nullptr;
  }

  void swap( function& other ) noexcept { 

    std::swap( *this, other ); 
  }

  bool operator==( function const& r ) const noexcept {

    return (object == r.object) && (wraper == r.wraper);
  }

  bool operator!=( function const& r ) const noexcept {

    return !operator==(r);
  }

  bool operator<( function const& r ) const noexcept {

    return (object < r.object) || ((object == r.object) && (wraper < r.wraper));
  }

  bool operator==( std::nullptr_t const ) const noexcept {

    return !wraper;
  }

  bool operator!=( std::nullptr_t const ) const noexcept {

    return wraper;
  }

  explicit operator bool() const noexcept { 

    return wraper; 
  }

  R operator()( A... args ) const {

    std::cout << "R operator()( A... args ) const" << std::endl;
    return wraper( object, std::forward<A>(args)... );
  }
};

}

#endif  //_FUNCTION_
