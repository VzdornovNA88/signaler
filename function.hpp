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

namespace signals {

template < typename T > class function;

template< class R, class ...A >
class function< R (A...) > final {

  using wraper_t      = R (*)( void*, A&&... );
  using deleter_t     = void (*)( void* );
  using ref_counter_t = unsigned char;


  /// Function content
  void*       object  = nullptr;
  wraper_t    wraper  = nullptr;
  void*       store   = nullptr;

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

    return f( std::forward<A>(args)... );
  }

  template < class T, R (T::*m)(A...) >
  static R m_wraper( void* const o, A&&... args ) {

    return (static_cast<T*>(o)->*m)( std::forward<A>(args)... );
  }

  template < class T, R (T::*m)(A...) const >
  static R m_wraper_const(void* const o, A&&... args) {

    return (static_cast<T const*>(o)->*m)( std::forward<A>(args)... );
  }

  template <typename T>
  static typename std::enable_if< !(is_pair<T>::value || 
                                    is_const_pair<T>::value), R >::type
  call_operator(void* const o, A&&... args) {

    return (*static_cast<T*>(o))(std::forward<A>(args)...);
  }

  template <typename T>
  static typename std::enable_if< is_pair<T>::value ||                  
                                  is_const_pair<T>::value, R >::type
  call_operator(void* const _o, A&&... args) {

    auto o = static_cast<T*>(_o)->first;
    auto m = static_cast<T*>(_o)->second;

    return (o->*m)( std::forward<A>(args)... );
  }


/// helpers
  static auto get_ref_counter( void* _store ) {

    return static_cast<ref_counter_t*> ( _store );
  }

  static auto get_deleter( void* _store ) {

    return static_cast<deleter_t*>     (
           static_cast<void*>          ( 
           static_cast<ref_counter_t*> ( _store ) + 1) );
  }

  template < class T >
  static auto get_store( void* _store ) {

    return static_cast< T* >           (
           static_cast<void*>          (
           static_cast<deleter_t*>     (
           static_cast<void*>          ( 
           static_cast<ref_counter_t*> ( _store ) + 1) ) + 1) );
  }

  template < class T >
  static void deleter( void* _store ) {

    get_ref_counter( _store )->~ref_counter_t();
    get_store<T>   ( _store )->~T();
          
    operator delete (_store);
    _store = nullptr;
  }
  
  void destructor() {

    if( store == nullptr ) return;

    ref_counter_t ref_cnt = *get_ref_counter( store );

    if( ref_cnt == 0 ) {

      (*get_deleter( store ))( store );
    }
    else {

      --ref_cnt;
      store = nullptr;
    }
  }

public:


/// destructor
  ~function() { destructor(); }


/// constructors
  function() = default;

  function( function const& f ) {

    destructor();

    object = f.object;
    wraper = f.wraper;
    store  = f.store;

    if( store != nullptr )
      ++(*get_ref_counter( store ));
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
             typename = typename ::std::enable_if< !::std::is_same<function, 
                        typename ::std::decay<T>::type>::value >::type >
  function(T&& f) : 
  store( operator new(  sizeof( ref_counter_t )                + 
                        sizeof( deleter_t )                    +
                        sizeof( typename std::decay<T>::type ) ) ) {

    using functor_t = typename std::decay<T>::type;

    new ( get_ref_counter( store ) ) ref_counter_t( 0 );

    auto del = static_cast<void*>(get_deleter( store ));
    new ( del ) deleter_t( deleter< functor_t > );

    auto functor = static_cast<void*>(get_store<functor_t>( store ));
    new ( functor ) functor_t( std::forward<T>(f) );

    object = functor;
    wraper = call_operator< functor_t >;
  }


/// copy operator
  function& operator = ( function const& f ) {

    destructor();

    object = f.object;
    wraper = f.wraper;
    store  = f.store;

    if( store != nullptr )
      ++(*get_ref_counter( store ));

    return *this;
  }


/// assignment operators
  template < class T >
  function& operator = ( R (T::* const m)(A...) ) {

    return *this = bind( static_cast<T*>(object), m );
  }

  template < class T >
  function& operator=( R (T::* const m)(A...) const ) {

    return *this = bind( static_cast<T const*>(object), m );
  }

  template < typename T,
             typename = typename ::std::enable_if<!::std::is_same<function, 
                        typename ::std::decay<T>::type>::value >::type  >
  function& operator = ( T&& f ) {

    using functor_t = typename std::decay<T>::type;

    destructor();

    store = operator new( sizeof( ref_counter_t ) + 
                          sizeof( deleter_t )     +
                          sizeof( functor_t )     );

    new ( get_ref_counter( store ) ) ref_counter_t( 0 );

    auto del = static_cast<void*>(get_deleter( store ));
    new ( del ) deleter_t( deleter< functor_t > );

    auto functor = static_cast<void*>(get_store<functor_t>( store ));
    new ( functor ) functor_t( std::forward<T>(f) );

    object = functor;
    wraper = call_operator< functor_t >;

    return *this;
  }

  function& operator = ( std::nullptr_t const null_object ) {

    destructor();
    return *this = bind( null_object );
  }

  function& operator = ( int const null_object ) {

    destructor();
    return *this = bind( null_object );
  }


/// binders
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


/// swap 
  void swap( function& other ) noexcept { 

    std::swap( *this, other ); 
  }


/// comparison operators
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


/// conversion to bool
  explicit operator bool() const noexcept { 

    return wraper; 
  }


/// callable
  R operator()( A... args ) const {

    return wraper( object, std::forward<A>(args)... );
  }
};

}

#endif  //_FUNCTION_
