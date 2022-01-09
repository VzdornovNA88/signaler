/**
 ******************************************************************************
 * @file             result.hpp
 * @brief            Simple pattern returned state - result
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

#ifndef SIGNALER_RESULT_HPP
#define SIGNALER_RESULT_HPP

#include <system_error>
#include <type_traits>
#include <variant>

namespace signaler {

template <typename T> class result_t {

  static_assert(std::is_nothrow_default_constructible<T>::value,
                "The type T does not satisfy is_nothrow_default_constructible "
                "as type of result_t");
  static_assert(std::is_nothrow_copy_constructible<T>::value,
                "The type T does not satisfy is_nothrow_copy_constructible as "
                "type of result_t");
  static_assert(std::is_nothrow_move_constructible<T>::value,
                "The type T does not satisfy is_nothrow_move_constructible as "
                "type of result_t");
  static_assert(std::is_nothrow_copy_assignable<T>::value,
                "The type T does not satisfy is_nothrow_copy_assignable as "
                "type of result_t");
  static_assert(std::is_nothrow_move_assignable<T>::value,
                "The type T does not satisfy is_nothrow_move_assignable as "
                "type of result_t");
  static_assert(std::is_nothrow_destructible<T>::value,
                "The type T does not satisfy is_nothrow_destructible as type "
                "of result_t");
  static_assert(!std::is_lvalue_reference<T>::value,
                "The type T does not satisfy NOT is_lvalue_reference as type "
                "of result_t");

  enum thing_t__ : unsigned char {
    VALUE = 0,
    ERROR = 1,
  };
  std::variant<T, std::error_code> storage_;

public:
  result_t(T &&v_) noexcept : storage_(std::forward<T>(v_)) {}

  result_t(T &v_) noexcept : storage_(v_) {}

  result_t &operator=(T &&v_) noexcept {
    storage_ = std::forward<T>(v_);
    return *this;
  };

  result_t &operator=(T &v_) noexcept {
    storage_ = v_;
    return *this;
  };

  result_t(std::error_code &s_) noexcept : storage_(s_) {}

  result_t &operator=(std::error_code &s_) noexcept { storage_ = s_; };

  result_t(std::error_code s_) noexcept : storage_(s_) {}

  result_t &operator=(std::error_code s_) noexcept {
    storage_ = s_;
    return *this;
  };

  result_t() noexcept = default;
  ~result_t() noexcept = default;
  result_t(const result_t &) noexcept = default;
  result_t(result_t &&) noexcept = default;
  result_t &operator=(const result_t &) noexcept = default;
  result_t &operator=(result_t &&) noexcept = default;

  template <typename callback_t>
  auto then(callback_t &&callback_) const noexcept {
    using ret_t__ = typename std::invoke_result<callback_t, T>::type;
    static_assert(!std::is_same_v<ret_t__, void>,
                  "callback_t shall not return void type");
    static_assert(std::is_invocable_r_v<ret_t__, callback_t, T>,
                  "callback_t type is not invocable with argument = [T]");

    if ((storage_.index() == VALUE))
      return result_t<ret_t__>{callback_(*std::get_if<VALUE>(&storage_))};
    else
      return result_t<ret_t__>{error()};
  }

  template <typename callback_t>
  auto catch_error(callback_t &&callback_) const noexcept {
    using ret_t__ =
        typename std::invoke_result<callback_t, std::error_code>::type;
    static_assert(std::is_same_v<ret_t__, void>,
                  "callback_t shall return void type");
    static_assert(std::is_invocable_r_v<ret_t__, callback_t, std::error_code>,
                  "callback_t type is not invocable with argument = [T]");

    if (!(storage_.index() == VALUE)) {
      auto error_ = *std::get_if<ERROR>(&storage_);
      callback_(error_);
      return result_t<ret_t__>{std::error_code{error_}};
    } else
      return result_t<ret_t__>{};
  }

  auto value() const noexcept {
    if (storage_.index() == VALUE)
      return *std::get_if<VALUE>(&storage_);
    else
      return T{};
  }
  auto error() const noexcept {
    if (storage_.index() == ERROR)
      return *std::get_if<ERROR>(&storage_);
    else
      return std::error_code{};
  }
  operator bool() const noexcept { return (storage_.index() == VALUE); }

  bool operator==(const result_t<T> &r) const noexcept {

    return ((storage_.index() == r.storage_.index()) &&
            (storage_ == r.storage_));
  }
};

template <> class result_t<void> {

  std::error_code status_;

public:
  result_t(std::error_code s_) noexcept : status_(s_) {}
  result_t(std::error_code &s_) noexcept : status_(s_) {}

  result_t() noexcept = default;
  ~result_t() noexcept = default;
  result_t(const result_t &) noexcept = default;
  result_t(result_t &&) noexcept = default;
  result_t &operator=(const result_t &) noexcept = default;
  result_t &operator=(result_t &&) noexcept = default;

  result_t &operator=(std::error_code s_) noexcept {
    status_ = s_;
    return *this;
  };
  result_t &operator=(std::error_code &s_) noexcept {
    status_ = s_;
    return *this;
  };

  auto error() const noexcept { return status_; }
  operator bool() const noexcept { return !status_; }

  template <typename callback_t>
  auto catch_error(callback_t &&callback_) const noexcept {
    using ret_t__ =
        typename std::invoke_result<callback_t, std::error_code>::type;
    static_assert(std::is_same_v<ret_t__, void>,
                  "callback_t shall return void type");
    static_assert(std::is_invocable_r_v<ret_t__, callback_t, std::error_code>,
                  "callback_t type is not invocable with argument = [T]");

    callback_(status_);
    return result_t<ret_t__>{status_};
  }

  bool operator==(const result_t<void> &r) const noexcept {

    return ((status_ == r.status_));
  }
};
} // namespace signaler

#endif // SIGNALER_RESULT_HPP