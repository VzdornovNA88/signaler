/**
 ******************************************************************************
 * @file             queue_traits.hpp
 * @brief            Traits for queue
 * @author           Nik A. Vzdornov (VzdornovNA88@yandex.ru)
 * @date             10.09.19
 * @copyright
 *
 * Copyright (c) 2021 VzdornovNA88
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
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

#ifndef SIGNALER_QUEUE_TRAITS_HPP
#define SIGNALER_QUEUE_TRAITS_HPP

#include "../../../result/result.hpp"
#include "queue_status.hpp"

namespace signaler::detail {

namespace {

using ret_type_for_push__ = result_t<void>;
template <typename T> using ret_type_for_wait_pop__ = result_t<T>;
using ret_type_for_empty__ = result_t<bool>;
using ret_type_for_max_size__ = size_t;

template <typename, typename = std::void_t<>>
struct has_queue_value_type : std::false_type {};

template <typename T>
struct has_queue_value_type<T, std::void_t<typename T::value_type>>
    : std::true_type {};

template <typename, typename = void> struct has_queue_push : std::false_type {};

template <typename T>
struct has_queue_push<
    T, ::std::enable_if_t<
           ::std::is_same<ret_type_for_push__,
                          decltype(std::declval<T>().push(
                              std::declval<typename T::value_type>()))>::value>>
    : std::true_type {};

template <typename, typename = void>
struct has_queue_wait_pop : std::false_type {};

template <typename T>
struct has_queue_wait_pop<
    T, ::std::enable_if_t<
           ::std::is_same<ret_type_for_wait_pop__<typename T::value_type>,
                          decltype(std::declval<T>().wait_pop())>::value>>
    : std::true_type {};

template <typename, typename = void>
struct has_queue_empty : std::false_type {};

template <typename T>
struct has_queue_empty<
    T, ::std::enable_if_t<::std::is_same<
           ret_type_for_empty__, decltype(std::declval<T>().empty())>::value>>
    : std::true_type {};

template <typename, typename = void>
struct has_queue_max_size : std::false_type {};

template <typename T>
struct has_queue_max_size<
    T, ::std::enable_if_t<
           ::std::is_same<ret_type_for_max_size__,
                          decltype(std::declval<T>().max_size())>::value>>
    : std::true_type {};

template <typename, typename = void> struct is_queue : std::false_type {};

template <typename T>
struct is_queue<
    T, std::enable_if_t<std::conjunction_v<

           has_queue_value_type<T>, has_queue_push<T>, has_queue_wait_pop<T>,
           has_queue_empty<T>, has_queue_max_size<T>,

           std::is_nothrow_default_constructible<typename T::value_type>,
           std::is_nothrow_copy_constructible<typename T::value_type>,
           std::is_nothrow_move_constructible<typename T::value_type>,
           std::is_nothrow_copy_assignable<typename T::value_type>,
           std::is_nothrow_move_assignable<typename T::value_type>,
           std::is_nothrow_destructible<typename T::value_type>

           >>> : std::true_type {};

template <typename T>
using template_parameter_is_queue =
    typename std::enable_if<is_queue<T>::value>::type;

using is_not_queue = void;
} // namespace

template <typename T, typename E = is_not_queue>
struct require_queue_concept_for {

  using queue_t = E;

  static_assert(has_queue_value_type<T>::value,
                "The type T does not satisfy the concept 'queue' ! It should "
                "contain: 'public: using value_type = T'");

  static_assert(has_queue_push<T>::value,
                "The type T does not satisfy the concept 'queue' ! It should "
                "contain: 'public: result<void> push(value_type&&) {...}'");

  static_assert(
      has_queue_wait_pop<T>::value,
      "The type T does not satisfy the concept 'queue' ! It should contain: "
      "'public: result_t<queue_t::value_type> wait_pop() {...}'");

  static_assert(has_queue_empty<T>::value,
                "The type T does not satisfy the concept 'queue' ! It should "
                "contain: 'public: result_t<bool> empty() const {...}'");

  static_assert(has_queue_max_size<T>::value,
                "The type T does not satisfy the concept 'queue' ! It should "
                "contain: 'public: size_t max_size() const {...}'");

  static_assert(
      std::is_nothrow_default_constructible<typename T::value_type>::value,
      "The type T does not satisfy is_nothrow_default_constructible ");

  static_assert(
      std::is_nothrow_copy_constructible<typename T::value_type>::value,
      "The type T does not satisfy is_nothrow_copy_constructible ");

  static_assert(
      std::is_nothrow_move_constructible<typename T::value_type>::value,
      "The type T does not satisfy is_nothrow_move_constructible ");

  static_assert(std::is_nothrow_copy_assignable<typename T::value_type>::value,
                "The type T does not satisfy is_nothrow_copy_assignable ");

  static_assert(std::is_nothrow_move_assignable<typename T::value_type>::value,
                "The type T does not satisfy is_nothrow_move_assignable ");

  static_assert(std::is_nothrow_destructible<typename T::value_type>::value,
                "The type T does not satisfy is_nothrow_destructible ");
};

template <typename T>
struct require_queue_concept_for<T, template_parameter_is_queue<T>> {

  using queue_t = T;
};

template <typename T> struct queue_traits {

  using ret_type_for_push_t = ret_type_for_push__;
  using ret_type_for_wait_pop_t =
      ret_type_for_wait_pop__<typename T::value_type>;
  using ret_type_for_empty_t = ret_type_for_empty__;
  using ret_type_for_max_size_t = ret_type_for_max_size__;
};
} // namespace signaler::detail

#endif