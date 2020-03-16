/**
  ******************************************************************************
  * @file             storage.hpp
  * @brief            
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

#ifndef _STORAGE_
#define _STORAGE_

namespace signals {
namespace detail  {

class storage_t final {

  using deleter_t     = void (*)( void* );
  using ref_counter_t = size_t;

  void* store = nullptr;

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
  
public:

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

  storage_t() = default;
  storage_t( std::nullptr_t const ) : storage_t() {}
  ~storage_t() { destructor(); }

  storage_t( storage_t const& s ) {

    destructor();

    store = s.store;

    if( store != nullptr )
      ++(*get_ref_counter( store ));
  }

  storage_t( storage_t&& s ) {

    destructor();

    store   = s.store;
    s.store = nullptr;
  }

  storage_t& operator = ( storage_t const& s ) {

    destructor();

    store = s.store;

    if( store != nullptr )
      ++(*get_ref_counter( store ));

    return *this;
  }

  storage_t& operator = ( storage_t&& s ) {

    destructor();

    store   = s.store;
    s.store = nullptr;

    return *this;
  }

  storage_t& operator = ( std::nullptr_t const null_object ) {

    destructor();
    return *this;
  }

  storage_t& operator = ( int const null_object ) {

    destructor();
    return *this;
  }

  template< typename T >
  auto make( T&& f ) {

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

    return functor;
  }

  operator void*() const {
    return store;
  }
};

}
}

#endif  //_STORAGE_