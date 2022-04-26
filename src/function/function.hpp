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

template <typename T> struct function_base_t__ {
  template <typename> static constexpr bool always_false_v = false;
  static_assert(always_false_v<T>,
                "[signaler] : signature of callable object does not satisfy "
                "'R(A...) CONST/VOLATILE/REF/NOEXCEPT'");
};

#define DEFINE_BASE_FUNCTION_VARIANT(CONST, VOLATILE, NOEXCEPT, REF)           \
  template <template <typename, size_t, atomicity_policy_t> class DERIVED,     \
            size_t SMALL_OPT_SIZE, atomicity_policy_t ATOMICITY_POLICY,        \
            typename R, typename... A>                                         \
  struct function_base_t__<DERIVED<R(A...) CONST VOLATILE REF NOEXCEPT,        \
                                   SMALL_OPT_SIZE, ATOMICITY_POLICY>> {        \
                                                                               \
    /*//! hotfix : msvc in vs2017(latest) doesn't compile static_assert with   \
     * fold expression*/                                                       \
    template <bool r> struct check_t__ { static constexpr bool value = r; };   \
                                                                               \
    static_assert(check_t__<(std::is_nothrow_move_constructible<A>::value &&   \
                             ...)>::value,                                     \
                  "[signaler] : The type T of one of the function arguments "  \
                  "does not satisfy the requirements : "                       \
                  "is_nothrow_move_constructible as argument of function_t");  \
    static_assert(check_t__<((std::is_rvalue_reference_v<A> ||                 \
                              std::is_nothrow_copy_constructible<A>::value) && \
                             ...)>::value,                                     \
                  "[signaler] : The type T of one of the function arguments "  \
                  "does not satisfy the requirements : "                       \
                  "is_nothrow_copy_constructible as argument of function_t");  \
                                                                               \
    struct traits_t__ {                                                        \
      using signature_t__ = R(A...) CONST VOLATILE REF NOEXCEPT;               \
      using derived_t__ =                                                      \
          DERIVED<signature_t__, SMALL_OPT_SIZE, ATOMICITY_POLICY>;            \
      using result_type_t__ = R;                                               \
      using args_t__ = std::tuple<A...>;                                       \
      using invoke_t__ = result_t<R> (*)(derived_t__ *const,                   \
                                         A &&...) noexcept;                    \
      template <typename T>                                                    \
      using const_ptr_method_of__ =                                            \
          R (T::*const)(A...) CONST VOLATILE REF NOEXCEPT;                     \
      template <typename T>                                                    \
      using ptr_method_of__ = R (T::*)(A...) CONST VOLATILE REF NOEXCEPT;      \
      using ptr_function_t__ = R (*)(A...) NOEXCEPT;                           \
      using const_ptr_function_t__ = R (*const)(A...) NOEXCEPT;                \
      template <typename, typename = std::void_t<>>                            \
      struct is_function_object_of__ : std::false_type {};                     \
      template <typename T>                                                    \
      struct is_function_object_of__<T, decltype(std::declval<T>().operator()( \
                                            std::declval<A>()...))>            \
          : std::true_type {};                                                 \
    };                                                                         \
                                                                               \
    typename traits_t__::derived_t__ *derived() CONST VOLATILE REF noexcept {  \
      return static_cast<typename traits_t__::derived_t__ *>(                  \
          const_cast<function_base_t__ *const>(this));                         \
    }                                                                          \
                                                                               \
    template <function_status_t status>                                        \
    static result_t<typename traits_t__::result_type_t__>                      \
    invoke([[maybe_unused]] typename traits_t__::derived_t__ *const,           \
           [[maybe_unused]] A &&...args) noexcept {                            \
                                                                               \
      return std::error_code{status};                                          \
    }                                                                          \
                                                                               \
    typename traits_t__::invoke_t__ invoker{                                   \
        invoke<function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>}; \
                                                                               \
    result_t<R> operator()(A... args) CONST VOLATILE REF noexcept {            \
      return invoker(derived(), std::forward<A>(args)...);                     \
    }                                                                          \
  };

DEFINE_BASE_FUNCTION_VARIANT(const, volatile, noexcept, &)

DEFINE_BASE_FUNCTION_VARIANT(const, volatile, noexcept, )
DEFINE_BASE_FUNCTION_VARIANT(const, volatile, , &)
DEFINE_BASE_FUNCTION_VARIANT(const, , noexcept, &)
DEFINE_BASE_FUNCTION_VARIANT(, volatile, noexcept, &)

DEFINE_BASE_FUNCTION_VARIANT(const, , , )
DEFINE_BASE_FUNCTION_VARIANT(const, volatile, , )
DEFINE_BASE_FUNCTION_VARIANT(const, , noexcept, )
DEFINE_BASE_FUNCTION_VARIANT(const, , , &)

DEFINE_BASE_FUNCTION_VARIANT(, volatile, , )
DEFINE_BASE_FUNCTION_VARIANT(, volatile, noexcept, )
DEFINE_BASE_FUNCTION_VARIANT(, volatile, , &)

DEFINE_BASE_FUNCTION_VARIANT(, , noexcept, )
DEFINE_BASE_FUNCTION_VARIANT(, , noexcept, &)

DEFINE_BASE_FUNCTION_VARIANT(, , , &)

DEFINE_BASE_FUNCTION_VARIANT(, , , )

DEFINE_BASE_FUNCTION_VARIANT(const, volatile, noexcept, &&)

DEFINE_BASE_FUNCTION_VARIANT(const, volatile, , &&)
DEFINE_BASE_FUNCTION_VARIANT(const, , noexcept, &&)
DEFINE_BASE_FUNCTION_VARIANT(, volatile, noexcept, &&)

DEFINE_BASE_FUNCTION_VARIANT(const, , , &&)

DEFINE_BASE_FUNCTION_VARIANT(, volatile, , &&)

DEFINE_BASE_FUNCTION_VARIANT(, , noexcept, &&)

DEFINE_BASE_FUNCTION_VARIANT(, , , &&)

template <typename SIGNATURE, size_t SMALL_OPT_SIZE,
          atomicity_policy_t ATOMICITY_POLICY>
class function_t__ final
    : public function_base_t__<
          function_t__<SIGNATURE, SMALL_OPT_SIZE, ATOMICITY_POLICY>> {

  template <typename T>
  using is_not_function_t__ = typename ::std::enable_if<!::std::is_same<
      function_t__, typename ::std::decay<T>::type>::value>::type;

  using base_t__ = function_base_t__<function_t__>;

public:
  using traits_t = typename base_t__::traits_t__;

private:
  using storage_type_t__ =
      detail::storage_t__<SMALL_OPT_SIZE, ATOMICITY_POLICY>;
  mutable storage_type_t__ store{nullptr};

  template <typename traits_t::ptr_function_t__ f, typename... A>
  static result_t<typename traits_t::result_type_t__>
  invoke([[maybe_unused]] typename traits_t::derived_t__ *const,
         A &&...args) noexcept {

    static_assert(
        f != nullptr,
        "[signaler] : Error in signaller::function_t<R(A...)>::template < "
        "R(*f)(A...) > static result_t<R> invoke([[maybe_unused]] "
        "function_t__* const, A&&... args) "
        "noexcept - The pointer 'f' to source function is null !");

    if constexpr (std::is_same_v<typename traits_t::result_type_t__, void>) {
      f(std::forward<A>(args)...);
      return std::error_code{function_status_t::F_CALL_SUCCESS};
    } else
      return {f(std::forward<A>(args)...)};
  }

  template <typename T, typename traits_t::template ptr_method_of__<T> m,
            typename... A>
  static result_t<typename traits_t::result_type_t__>
  invoke(typename traits_t::derived_t__ *const self, A &&...args) noexcept {

    static_assert(
        m != nullptr,
        "[signaler] : Error in signaller::function_t<R(A...)>::template < "
        "R(T::*m)(A...) > "
        "static result_t<R> invoke(function_t__* const, "
        "A&&... args) noexcept - The pointer to function member 'm' of type T "
        "is null !");

    if constexpr (std::is_same_v<typename traits_t::result_type_t__, void>) {
      ((self->store.template get<T *>())->*m)(std::forward<A>(args)...);
      return std::error_code{function_status_t::F_CALL_SUCCESS};
    } else
      return {
          ((self->store.template get<T *>())->*m)(std::forward<A>(args)...)};
  }

  template <typename T, typename... A>
  static result_t<typename traits_t::result_type_t__>
  invoke(typename traits_t::derived_t__ *const self, A &&...args) noexcept {

    using functor_t__ = typename std::conditional<
        std::is_function<typename std::remove_reference<T>::type>::value,
        typename std::add_pointer<
            typename std::remove_reference<T>::type>::type,
        typename std::remove_reference<T>::type>::type;

    if constexpr (std::is_same_v<typename traits_t::result_type_t__, void>) {
      (*self->store.template get<functor_t__>())(std::forward<A>(args)...);
      return std::error_code{function_status_t::F_CALL_SUCCESS};
    } else
      return {
          (*self->store.template get<functor_t__>())(std::forward<A>(args)...)};
  }

  function_t__(void *const o, typename traits_t::invoke_t__ const m) noexcept
      : base_t__{m}, store{o} {}

  function_t__(std::nullptr_t const o,
               typename traits_t::invoke_t__ const m) noexcept
      : base_t__{m}, store{o} {}

public:
  function_t__() noexcept {};
  ~function_t__() noexcept {};

  function_t__(std::nullptr_t const) noexcept {}
  function_t__(int const) noexcept {}

  template <typename T, typename = is_not_function_t__<T>>
  function_t__(T &&f, memmory_resource_noexcept_t *res = nullptr) noexcept(
      noexcept(std::move(std::declval<T>())))
      : store(std::forward<T>(f), res) {

    if (store != nullptr)
      base_t__::invoker = invoke<T>;
  }

  function_t__(function_t__ const &f) noexcept {

    base_t__::invoker = f.invoker;
    store = f.store;
  }

  function_t__(function_t__ &&f) noexcept {

    base_t__::invoker = f.invoker;
    store = ::std::move(f.store);

    f.invoker = base_t__::template invoke<
        function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;
  }

  function_t__ &operator=(function_t__ const &f) noexcept {

    base_t__::invoker = f.invoker;
    store = f.store;

    return *this;
  }

  function_t__ &operator=(function_t__ &&f) noexcept {

    base_t__::invoker = f.invoker;
    store = ::std::move(f.store);

    f.invoker = base_t__::template invoke<
        function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;

    return *this;
  }

  template <typename T, typename = is_not_function_t__<T>>
  function_t__ &
  operator=(T &&f) noexcept(noexcept(std::move(std::declval<T>()))) {

    store = std::forward<T>(f);

    if (store != nullptr)
      base_t__::invoker = invoke<T>;
    else
      base_t__::invoker = base_t__::template invoke<
          function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;

    return *this;
  }

  function_t__ &operator=(std::nullptr_t const null_object) noexcept {

    return *this = bind(null_object);
  }

  function_t__ &operator=(int const null_object) noexcept {

    return *this = bind(null_object);
  }

  template <typename traits_t::ptr_function_t__ f>
  static function_t__ bind() noexcept {

    return {nullptr, invoke<f>};
  }

  template <typename T, typename traits_t::template ptr_method_of__<T> m>
  static function_t__ bind(T *const o) noexcept {

    return {o, invoke<T, m>};
  }

  template <typename T, typename traits_t::template ptr_method_of__<T> m>
  static function_t__ bind(T &o) noexcept {

    return {&o, invoke<T, m>};
  }

  static function_t__ bind(std::nullptr_t const null_object) noexcept {

    return null_object;
  }

  static function_t__ bind(int const) noexcept { return nullptr; }

  bool operator==(function_t__ const &r) const noexcept {

    return (store == r.store) && (base_t__::invoker == r.invoker);
  }

  bool operator!=(function_t__ const &r) const noexcept {
    return !operator==(r);
  }

  bool operator==(std::nullptr_t const) const noexcept {

    return base_t__::invoker ==
           &base_t__::template invoke<
               function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;
  }

  bool operator!=(std::nullptr_t const) const noexcept {

    return base_t__::invoker !=
           &base_t__::template invoke<
               function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;
  }

  explicit operator bool() const noexcept {

    return base_t__::invoker !=
           &base_t__::template invoke<
               function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED>;
  }
};

} // namespace detail

template <typename SIGNATURE, size_t SMALL_OPT_SIZE = 16,
          detail::atomicity_policy_t ATOMICITY_POLICY =
              signaler::detail::atomicity_policy_t::NON_ATOMIC>
using function_t =
    detail::function_t__<SIGNATURE, SMALL_OPT_SIZE, ATOMICITY_POLICY>;

} // namespace signaler

#endif // SIGNALER_FUNCTION_HPP
