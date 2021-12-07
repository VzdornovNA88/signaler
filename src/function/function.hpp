/**
 ******************************************************************************
 * @file             function.hpp
 * @brief            Simple delegate pattern
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

#ifndef SIGNALER_FUNCTION_HPP
#define SIGNALER_FUNCTION_HPP

#include <type_traits>
#include <utility>

#include "../result/result.hpp"
#include "detail/storage.hpp"

namespace signaler {

enum class function_status_t : unsigned char {

  F_CALL_SUCCESS = 0,
  F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED = 1,
};
}

namespace std {
template <>
struct is_error_code_enum<signaler::function_status_t> : true_type {};
} // namespace std

namespace signaler {

std::error_code make_error_code(function_status_t);

namespace detail {

template <typename T, size_t SMALL_OPT_SIZE /*= 16*/> class function_t__;


#define DEFINE_FUNCTION_VARIANT(CONST, VOLATILE, NOEXCEPT, REF)                                                                                    \
template <typename R, size_t SMALL_OPT_SIZE, typename... A>                                                                                        \
class function_t__<R(A...) CONST VOLATILE REF NOEXCEPT, SMALL_OPT_SIZE> final {                                                                    \
                                                                                                                                                   \
  /*//! hotfix : msvc in vs2017(latest) doesn't compile static_assert with fold expression*/                                                       \
  template<bool r>                                                                                                                                 \
	struct check_t__ {                                                                                                                               \
		static constexpr bool value = r;                                                                                                               \
	};                                                                                                                                               \
                                                                                                                                                   \
  static_assert(check_t__<(std::is_nothrow_move_constructible<A>::value && ...)>::value ,                                                          \
                "The type T of one of the function arguments does not satisfy the requirements : "                                                 \
                "is_nothrow_move_constructible as argument of function_t");                                                                        \
  static_assert(check_t__<((std::is_rvalue_reference_v<A> || std::is_nothrow_copy_constructible<A>::value) && ...)>::value,                        \
                "The type T of one of the function arguments does not satisfy the requirements : "                                                 \
                "is_nothrow_copy_constructible as argument of function_t");                                                                        \
                                                                                                                                                   \
  using wraper_t__ = result_t<R> (*)(CONST VOLATILE detail::storage_t__<SMALL_OPT_SIZE> *const,                                                    \
                                   A &&...) noexcept;                                                                                              \
                                                                                                                                                   \
  template <typename T>                                                                                                                            \
  using is_not_function_t__ = typename ::std::enable_if<                                                                                           \
      !::std::is_same<function_t__, typename ::std::decay<T>::type>::value>::type;                                                                 \
                                                                                                                                                   \
  wraper_t__ aplly {p_aplly<function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>};                                                        \
  mutable detail::storage_t__<SMALL_OPT_SIZE> store {nullptr};                                                                                     \
                                                                                                                                                   \
  template <function_status_t status>                                                                                                              \
  static result_t<R>                                                                                                                               \
  p_aplly([[maybe_unused]] CONST VOLATILE detail::storage_t__<SMALL_OPT_SIZE> *const,                                                              \
          [[maybe_unused]] A &&...args) noexcept {                                                                                                 \
                                                                                                                                                   \
    return {status};                                                                                                                               \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <R (*f)(A...) NOEXCEPT>                                                                                                                 \
  static result_t<R>                                                                                                                               \
  p_aplly([[maybe_unused]] CONST VOLATILE detail::storage_t__<SMALL_OPT_SIZE> *const,                                                              \
          A &&...args) noexcept {                                                                                                                  \
                                                                                                                                                   \
    static_assert(f != nullptr,                                                                                                                    \
                  "Error in signaller::function_t<R(A...)>::template < "                                                                           \
                  "R(*f)(A...) > static result_t<R> p_aplly([[maybe_unused]] "                                                                     \
                  "detail::storage_t__<SMALL_OPT_SIZE>* const, A&&... args) "                                                                      \
                  "noexcept - The pointer 'f' to source function is null !");                                                                      \
                                                                                                                                                   \
    if constexpr (std::is_same_v<R, void>) {                                                                                                       \
      f(std::forward<A>(args)...);                                                                                                                 \
      return {function_status_t::F_CALL_SUCCESS};                                                                                                  \
    } else                                                                                                                                         \
      return {f(std::forward<A>(args)...), function_status_t::F_CALL_SUCCESS};                                                                     \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T, R (T::*m)(A...) CONST VOLATILE REF NOEXCEPT>                                                                               \
  static result_t<R> p_aplly( CONST VOLATILE detail::storage_t__<SMALL_OPT_SIZE> *const s,                                                         \
                             A &&...args) noexcept {                                                                                               \
                                                                                                                                                   \
    static_assert(                                                                                                                                 \
        m != nullptr,                                                                                                                              \
        "Error in signaller::function_t<R(A...)>::template < R(T::*m)(A...) > "                                                                    \
        "static result_t<R> p_aplly(detail::storage_t__<SMALL_OPT_SIZE>* const, "                                                                  \
        "A&&... args) noexcept - The pointer to function member 'm' of type T "                                                                    \
        "is null !");                                                                                                                              \
                                                                                                                                                   \
    if constexpr (std::is_same_v<R, void>) {                                                                                                       \
      ((const_cast<detail::storage_t__<SMALL_OPT_SIZE> *const>(s)->template get<T *>())->*m)(std::forward<A>(args)...);                            \
      return {function_status_t::F_CALL_SUCCESS};                                                                                                  \
    } else                                                                                                                                         \
      return {((const_cast<detail::storage_t__<SMALL_OPT_SIZE> *const>(s)->template get<T *>())->*m)(std::forward<A>(args)...),                    \
              function_status_t::F_CALL_SUCCESS};                                                                                                  \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T>                                                                                                                            \
  static result_t<R> p_aplly(CONST VOLATILE detail::storage_t__<SMALL_OPT_SIZE> *const s,                                                          \
                             A &&...args) noexcept {                                                                                               \
                                                                                                                                                   \
    /*static_assert(std::is_nothrow_move_constructible<T>::value,*/                                                                                \
            /*"The type T does not satisfy is_nothrow_move_constructible ");*/                                                                     \
    /*static_assert(std::is_nothrow_destructible<T>::value,*/                                                                                      \
            /*"The type T does not satisfy is_nothrow_destructible ");*/                                                                           \
                                                                                                                                                   \
    if constexpr (std::is_same_v<R, void>) {                                                                                                       \
      (*const_cast<detail::storage_t__<SMALL_OPT_SIZE> *const>(s)->template get<T>())(std::forward<A>(args)...);                                   \
      return {function_status_t::F_CALL_SUCCESS};                                                                                                  \
    } else                                                                                                                                         \
      return {(*const_cast<detail::storage_t__<SMALL_OPT_SIZE> *const>(s)->template get<T>())(std::forward<A>(args)...),                           \
              function_status_t::F_CALL_SUCCESS};                                                                                                  \
  }                                                                                                                                                \
                                                                                                                                                   \
  function_t__(void *const o, wraper_t__ const m) noexcept : aplly(m),store(o){}                                                                   \
                                                                                                                                                   \
  function_t__(std::nullptr_t const o, wraper_t__ const m) noexcept                                                                                \
      : aplly(m),store(o) {}                                                                                                                       \
                                                                                                                                                   \
public:                                                                                                                                            \
  function_t__() noexcept {};                                                                                                                      \
  ~function_t__() noexcept {};                                                                                                                     \
                                                                                                                                                   \
  function_t__(std::nullptr_t const) noexcept {}                                                                                                   \
  function_t__(int const) noexcept {}                                                                                                              \
                                                                                                                                                   \
                                                                                                                                                   \
  template <typename T, typename functor_t__ = typename std::decay<T>::type,                                                                       \
            typename = is_not_function_t__<T>>                                                                                                     \
  function_t__(T &&f) noexcept : store(std::forward<T>(f)) {                                                                                       \
                                                                                                                                                   \
    static_assert( std::is_invocable_r_v<R, std::remove_pointer_t<functor_t__>, A...>,                                                             \
                   "Type 'T' can not be invocable with these arguments !" );                                                                       \
                                                                                                                                                   \
    if (store != nullptr)                                                                                                                          \
      aplly = p_aplly<functor_t__>;                                                                                                                \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T> function_t__(T *const o, R (T::*const m)(A...) CONST VOLATILE REF NOEXCEPT) noexcept {                                     \
                                                                                                                                                   \
    *this = bind(o, m);                                                                                                                            \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T> function_t__(T &o, R (T::*const m)(A...) CONST VOLATILE REF NOEXCEPT) noexcept {                                           \
                                                                                                                                                   \
    *this = bind(o, m);                                                                                                                            \
  }                                                                                                                                                \
                                                                                                                                                   \
  function_t__(function_t__ const &f) noexcept {                                                                                                   \
                                                                                                                                                   \
    aplly = f.aplly;                                                                                                                               \
    store = f.store;                                                                                                                               \
  }                                                                                                                                                \
                                                                                                                                                   \
  function_t__(function_t__ &&f) noexcept {                                                                                                        \
                                                                                                                                                   \
    aplly = f.aplly;                                                                                                                               \
    store = ::std::move(f.store);                                                                                                                  \
                                                                                                                                                   \
    f.aplly =                                                                                                                                      \
        p_aplly<function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;                                                                     \
  }                                                                                                                                                \
                                                                                                                                                   \
  function_t__ &operator=(function_t__ const &f) noexcept {                                                                                        \
                                                                                                                                                   \
    aplly = f.aplly;                                                                                                                               \
    store = f.store;                                                                                                                               \
                                                                                                                                                   \
    return *this;                                                                                                                                  \
  }                                                                                                                                                \
                                                                                                                                                   \
  function_t__ &operator=(function_t__ &&f) noexcept {                                                                                             \
                                                                                                                                                   \
    aplly = f.aplly;                                                                                                                               \
    store = ::std::move(f.store);                                                                                                                  \
                                                                                                                                                   \
    f.aplly =                                                                                                                                      \
        p_aplly<function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;                                                                     \
                                                                                                                                                   \
    return *this;                                                                                                                                  \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T, typename = is_not_function_t__<T>>                                                                                         \
  function_t__ &operator=(T &&f) noexcept {                                                                                                        \
                                                                                                                                                   \
    using functor_t__ = typename std::decay<T>::type;                                                                                              \
    using base_type_fun_t__ = typename std::remove_pointer_t<functor_t__>;                                                                         \
    static_assert( std::is_invocable_r_v<R, base_type_fun_t__, A...>,                                                                              \
                   "Type 'T' can not be invocable with these arguments !" );                                                                       \
                                                                                                                                                   \
    static_assert( check_t__<!std::is_class_v<base_type_fun_t__> || /*std::is_nothrow_move_assignable_v<base_type_fun_t__> &&*/                    \
                             std::is_nothrow_destructible_v<base_type_fun_t__>>::value,                                                            \
                   "In function_t__ &operator=(T &&f) noexcept : "                                                                                 \
                   "Type 'T' does not satisfy the requirements : is_nothrow_move_assignable and is_nothrow_destructible !" );                      \
                                                                                                                                                   \
    store = std::forward<T>(f);                                                                                                                    \
                                                                                                                                                   \
    if (store != nullptr)                                                                                                                          \
      aplly = p_aplly<functor_t__>;                                                                                                                \
    else                                                                                                                                           \
      aplly =                                                                                                                                      \
          p_aplly<function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;                                                                   \
                                                                                                                                                   \
    return *this;                                                                                                                                  \
  }                                                                                                                                                \
                                                                                                                                                   \
  function_t__ &operator=(std::nullptr_t const null_object) noexcept {                                                                             \
                                                                                                                                                   \
    return *this = bind(null_object);                                                                                                              \
  }                                                                                                                                                \
                                                                                                                                                   \
  function_t__ &operator=(int const null_object) noexcept {                                                                                        \
                                                                                                                                                   \
    return *this = bind(null_object);                                                                                                              \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <R (*const f)(A...) NOEXCEPT> static function_t__ bind() noexcept {                                                                     \
                                                                                                                                                   \
    return {nullptr, p_aplly<f>};                                                                                                                  \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T, R (T::*const m)(A...) CONST VOLATILE REF NOEXCEPT>                                                                         \
  static function_t__ bind(T *const o) noexcept {                                                                                                  \
                                                                                                                                                   \
    return {o, p_aplly<T, m>};                                                                                                                     \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T, R (T::*const m)(A...) CONST VOLATILE REF NOEXCEPT>                                                                         \
  static function_t__ bind(T &o) noexcept {                                                                                                        \
                                                                                                                                                   \
    return {&o, p_aplly<T, m>};                                                                                                                    \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T>                                                                                                                            \
  static function_t__ bind(T *const o, R (T::*const m)(A...) CONST VOLATILE REF NOEXCEPT) noexcept {                                               \
                                                                                                                                                   \
    if (o && m)                                                                                                                                    \
      return                                                                                                                                       \
          [=](A &&...args) -> R { return (o->*m)(std::forward<A>(args)...); };                                                                     \
    else                                                                                                                                           \
      return nullptr;                                                                                                                              \
  }                                                                                                                                                \
                                                                                                                                                   \
  template <typename T>                                                                                                                            \
  static function_t__ bind(T &o, R (T::*const m)(A...) CONST VOLATILE REF NOEXCEPT) noexcept {                                                     \
                                                                                                                                                   \
    if (m)                                                                                                                                         \
      return [&o, m](A &&...args) -> R {                                                                                                           \
        return (o.*m)(std::forward<A>(args)...);                                                                                                   \
      };                                                                                                                                           \
    else                                                                                                                                           \
      return nullptr;                                                                                                                              \
  }                                                                                                                                                \
                                                                                                                                                   \
  static function_t__ bind(std::nullptr_t const null_object) noexcept {                                                                            \
                                                                                                                                                   \
    return null_object;                                                                                                                            \
  }                                                                                                                                                \
                                                                                                                                                   \
  static function_t__ bind(int const) noexcept { return nullptr; }                                                                                 \
                                                                                                                                                   \
  bool operator==(function_t__ const &r) const noexcept {                                                                                          \
                                                                                                                                                   \
    return (store == r.store) && (aplly == r.aplly);                                                                                               \
  }                                                                                                                                                \
                                                                                                                                                   \
  bool operator!=(function_t__ const &r) const noexcept { return !operator==(r); }                                                                 \
                                                                                                                                                   \
  bool operator==(std::nullptr_t const) const noexcept {                                                                                           \
                                                                                                                                                   \
    return aplly ==                                                                                                                                \
           &p_aplly<                                                                                                                               \
               function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;                                                                      \
  }                                                                                                                                                \
                                                                                                                                                   \
  bool operator!=(std::nullptr_t const) const noexcept {                                                                                           \
                                                                                                                                                   \
    return aplly !=                                                                                                                                \
           &p_aplly<                                                                                                                               \
               function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;                                                                      \
  }                                                                                                                                                \
                                                                                                                                                   \
  explicit operator bool() const noexcept {                                                                                                        \
                                                                                                                                                   \
    return aplly !=                                                                                                                                \
           &p_aplly<                                                                                                                               \
               function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;                                                                      \
  }                                                                                                                                                \
                                                                                                                                                   \
  result_t<R> operator()(A... args) CONST VOLATILE REF noexcept {                                                                                  \
                                                                                                                                                   \
    return aplly(&store, std::forward<A>(args)...);                                                                                                \
  }                                                                                                                                                \
};

DEFINE_FUNCTION_VARIANT(const, volatile, noexcept, &)

DEFINE_FUNCTION_VARIANT(const, volatile, noexcept,)
DEFINE_FUNCTION_VARIANT(const, volatile, , &)
DEFINE_FUNCTION_VARIANT(const, , noexcept, &)
DEFINE_FUNCTION_VARIANT(, volatile, noexcept, &)


DEFINE_FUNCTION_VARIANT(const, , ,)
DEFINE_FUNCTION_VARIANT(const, volatile, ,)
DEFINE_FUNCTION_VARIANT(const, , noexcept,)
DEFINE_FUNCTION_VARIANT(const, , , &) 

DEFINE_FUNCTION_VARIANT(, volatile, ,)
DEFINE_FUNCTION_VARIANT(, volatile, noexcept,)
DEFINE_FUNCTION_VARIANT(, volatile, , &) 

DEFINE_FUNCTION_VARIANT(, , noexcept,)
DEFINE_FUNCTION_VARIANT(, , noexcept, &)

DEFINE_FUNCTION_VARIANT(, , , &)

DEFINE_FUNCTION_VARIANT(, , ,) 


DEFINE_FUNCTION_VARIANT(const, volatile, noexcept, &&)

DEFINE_FUNCTION_VARIANT(const, volatile, , &&)
DEFINE_FUNCTION_VARIANT(const, , noexcept, &&)
DEFINE_FUNCTION_VARIANT(, volatile, noexcept, &&)

DEFINE_FUNCTION_VARIANT(const, , , &&) 

DEFINE_FUNCTION_VARIANT(, volatile, , &&) 

DEFINE_FUNCTION_VARIANT(, , noexcept, &&)

DEFINE_FUNCTION_VARIANT(, , , &&)          


} // namespace detail


template< typename SIGNATURE , size_t SMALL_OPT_SIZE = 16 >
using function_t = detail::function_t__<SIGNATURE,SMALL_OPT_SIZE>;


} // namespace signaler

#endif // SIGNALER_FUNCTION_HPP
