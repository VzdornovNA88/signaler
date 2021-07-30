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

namespace signaler {

template <typename T> class result_t {

  static_assert(
      std::is_nothrow_default_constructible<T>::value,
      "The type T does not satisfy is_nothrow_default_constructible as type of result_t");
  static_assert(std::is_nothrow_copy_constructible<T>::value,
                "The type T does not satisfy is_nothrow_copy_constructible as type of result_t");
  static_assert(std::is_nothrow_move_constructible<T>::value,
                "The type T does not satisfy is_nothrow_move_constructible as type of result_t");
  static_assert(std::is_nothrow_copy_assignable<T>::value,
                "The type T does not satisfy is_nothrow_copy_assignable as type of result_t");
  static_assert(std::is_nothrow_move_assignable<T>::value,
                "The type T does not satisfy is_nothrow_move_assignable as type of result_t");
  static_assert(std::is_nothrow_destructible<T>::value,
                "The type T does not satisfy is_nothrow_destructible as type of result_t");
  static_assert(!std::is_lvalue_reference<T>::value,
                "The type T does not satisfy NOT is_lvalue_reference as type of result_t");

  T val_{};
  std::error_code status_;

public:
  result_t(T &v_, std::error_code &s_) noexcept : val_(v_), status_(s_) {}

  result_t(T &&v_, std::error_code &s_) noexcept
      : val_(std::forward<T>(v_)), status_(s_) {}

  result_t(T &v_, std::error_code s_) noexcept : val_(v_), status_(s_) {}

  result_t(T &&v_, std::error_code s_) noexcept
      : val_(std::forward<T>(v_)), status_(s_) {}

  result_t(T &&v_) noexcept : val_(std::forward<T>(v_)) {}

  result_t(T &v_) noexcept : val_(v_) {}

  result_t &operator=(T &&v_) noexcept {
    val_ = std::forward<T>(v_);
    return *this;
  };

  result_t &operator=(T &v_) noexcept {
    val_ = v_;
    return *this;
  };

  result_t(std::error_code &s_) noexcept : status_(s_) {}

  result_t &operator=(std::error_code &s_) noexcept { status_ = s_; };

  result_t(std::error_code s_) noexcept : status_(s_) {}

  result_t &operator=(std::error_code s_) noexcept {
    status_ = s_;
    return *this;
  };

  result_t() noexcept = default;
  ~result_t() noexcept = default;
  result_t(const result_t &) noexcept = default;
  result_t(result_t &&) noexcept = default;
  result_t &operator=(const result_t &) noexcept = default;
  result_t &operator=(result_t &&) noexcept = default;

  auto value() const noexcept { return val_; }
  auto status() const noexcept { return status_; }
  operator bool() const noexcept { return !status_; }

  bool operator==(const result_t<T> &r) const noexcept {

    return ((val_ == r.val_) && (status_ == r.status_));
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

  auto status() const noexcept { return status_; }
  operator bool() const noexcept { return !status_; }

  bool operator==(const result_t<void> &r) const noexcept {

    return ((status_ == r.status_));
  }
};
} // namespace signaler

#endif // SIGNALER_RESULT_HPP