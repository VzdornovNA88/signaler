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

template < typename T > class function_t;

template< typename R, typename ...A >
class function_t< R (A...) > final {

  using wraper_t      = R (*)( void*, A&&... );
  using deleter_t     = void (*)( void* );
  using ref_counter_t = size_t;


  /// Function content
  void*       object  = nullptr;
  wraper_t    wraper  = nullptr;
  void*       store   = nullptr;

  function_t( void* const o, wraper_t const m ) : 
  object(o),wraper(m) {}


  /// Private type traits
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


  /// Wrapers
  template < R (*f)(A...) >
  static R f_wraper( void* const, A&&... args ) {

    return f( std::forward<A>(args)... );
  }

  template < typename T, R (T::*m)(A...) >
  static R f_wraper( void* const o, A&&... args ) {

    return (static_cast<T*>(o)->*m)( std::forward<A>(args)... );
  }

  template < typename T, R (T::*m)(A...) const >
  static R f_wraper(void* const o, A&&... args) {

    return (static_cast<T const*>(o)->*m)( std::forward<A>(args)... );
  }

  template <typename T>
  static typename std::enable_if< !(is_pair<T>::value || 
                                    is_const_pair<T>::value), R >::type
  f_wraper(void* const o, A&&... args) {

    return (*static_cast<T*>(o))(std::forward<A>(args)...);
  }

  template <typename T>
  static typename std::enable_if< is_pair<T>::value ||                  
                                  is_const_pair<T>::value, R >::type
  f_wraper(void* const _o, A&&... args) {

    auto o = static_cast<T*>(_o)->first;
    auto m = static_cast<T*>(_o)->second;

    return (o->*m)( std::forward<A>(args)... );
  }

  static R f_wraper(void* const o, A&&... args) {

    return (*reinterpret_cast<R(*)(A...)>(o))(std::forward<A>(args)...);
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

  template < typename T >
  static auto get_store( void* _store ) {

    return static_cast< T* >           (
           static_cast<void*>          (
           static_cast<deleter_t*>     (
           static_cast<void*>          ( 
           static_cast<ref_counter_t*> ( _store ) + 1) ) + 1) );
  }

  template < typename T >
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
  ~function_t() { destructor(); }


/// constructors
  function_t() = default;

  function_t( function_t const& f ) {

    destructor();

    object = f.object;
    wraper = f.wraper;
    store  = f.store;

    if( store != nullptr )
      ++(*get_ref_counter( store ));
  }

  function_t( std::nullptr_t const ) : function_t() {}

  template < typename T, 
  typename = typename std::enable_if< std::is_class<T>::value >::type >
  explicit function_t( T const* const o ) : object( const_cast<T*>(o) ) {}

  template < typename T, 
  typename = typename std::enable_if< std::is_class<T>::value >::type >
  explicit function_t( T const& o ) : object( const_cast<T*>(&o) ) {}

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
             !::std::is_function<T>::value >::type  >
  function_t(T&& f) : 
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
    wraper = f_wraper< functor_t >;
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
              ::std::is_function<T>::value >::type  >
  function_t(T* f) {

    object = reinterpret_cast< void* >( f );
    wraper = f_wraper;
  }


/// copy operator
  function_t& operator = ( function_t const& f ) {

    destructor();

    object = f.object;
    wraper = f.wraper;
    store  = f.store;

    if( store != nullptr )
      ++(*get_ref_counter( store ));

    return *this;
  }


/// assignment operators
  template < typename T >
  function_t& operator = ( R (T::* const m)(A...) ) {

    return *this = bind( static_cast<T*>(object), m );
  }

  template < typename T >
  function_t& operator=( R (T::* const m)(A...) const ) {

    return *this = bind( static_cast<T const*>(object), m );
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
             !::std::is_function<T>::value >::type  >
  function_t& operator = ( T&& f ) {

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
    wraper = f_wraper< functor_t >;

    return *this;
  }

  template < typename T,
             typename = typename ::std::enable_if<
             !::std::is_same<function_t,typename ::std::decay<T>::type>::value &&
              ::std::is_function<T>::value >::type >
  function_t& operator = ( T* f ) {

    destructor();

    object = reinterpret_cast< void* >( f );
    wraper = f_wraper;

    return *this;
  }

  function_t& operator = ( std::nullptr_t const null_object ) {

    destructor();
    return *this = bind( null_object );
  }

  function_t& operator = ( int const null_object ) {

    destructor();
    return *this = bind( null_object );
  }


/// binders
  template < R (* const f)(A...) >
  static function_t bind() {

    return { nullptr, f_wraper<f> };
  }

  template < typename T, R (T::* const m)(A...) >
  static function_t bind( T* const o ) {

    return { o, f_wraper<T, m> };
  }

  template < typename T, R (T::* const m)(A...) const >
  static function_t bind( T const* const o ) {

    return { const_cast<T*>(o), f_wraper<T, m> };
  }

  template < typename T, R (T::* const m)(A...) >
  static function_t bind( T& o ) {

    return { &o, f_wraper<T, m> };
  }

  template < typename T, R (T::* const m)(A...) const >
  static function_t bind( T const& o ) {

    return { const_cast<T*>(&o), f_wraper<T, m> };
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


/// swap 
  void swap( function_t& other ) { 

    std::swap( *this, other ); 
  }


/// comparison operators
  bool operator==( function_t const& r ) const {

    return (object == r.object) && (wraper == r.wraper);
  }

  bool operator!=( function_t const& r ) const {

    return !operator==(r);
  }

  bool operator<( function_t const& r ) const {

    return (object < r.object) || ((object == r.object) && (wraper < r.wraper));
  }

  bool operator==( std::nullptr_t const ) const {

    return !wraper;
  }

  bool operator!=( std::nullptr_t const ) const {

    return wraper;
  }


/// conversion to bool
  explicit operator bool() const { 

    return wraper; 
  }


/// callable
  R operator()( A... args ) const {

    return wraper( object, std::forward<A>(args)... );
  }
};

}

#endif  //_FUNCTION_
