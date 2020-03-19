/**
  ******************************************************************************
  * @file             type_traits.hpp
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

#ifndef _TYPE_TRAITS_
#define _TYPE_TRAITS_

namespace signals {
namespace detail  {

template <typename,typename,typename = void>
class is_functor : public std::false_type {};

template <typename Type,typename Ret,typename... Args>
class is_functor<Type,Ret(Args...), 
typename std::enable_if< !::std::is_scalar<typename std::decay<Type>::type>::value >::type> {

  using clazz = typename std::decay<Type>::type;
	
  using _signature       = Ret(clazz::*)(Args...);
  using _signature_const = Ret(clazz::*)(Args...)const;
  
  template< typename F >
  using _get_signature_of       = decltype(static_cast<_signature>(&F::operator()));
  template< typename F >
  using _get_signature_const_of = decltype(static_cast<_signature_const>(&F::operator()));
  
  template< typename F >
  using _is_invokable = std::is_same<_get_signature_of<F>,_signature>;
  template< typename F >
  using _is_invokable_const = std::is_same<_get_signature_const_of<F>,_signature_const>;

  template <typename C,typename = void>
  struct _is_functor : std::false_type {};
  
  template <typename C>
  struct _is_functor<C, typename std::enable_if< 
                                 _is_invokable<C>::value >::type > 
                     : std::true_type  {};
                     
  template <typename C>
  struct _is_functor<C, typename std::enable_if< 
                                 _is_invokable_const<C>::value >::type > 
                     : std::true_type  {};                   

public:
  constexpr static bool value = _is_functor<clazz>::value;                       
};

}
}

#endif  //_TYPE_TRAITS_