/**
 ******************************************************************************
 * @file             id_connection_generator.hpp
 * @brief            Simple id generator for signal connection
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

#ifndef SIGNALER_ID_GEN_CON_HPP
#define SIGNALER_ID_GEN_CON_HPP

namespace signaler::detail {

class id_t__ {
public:
  using id_type_internal = unsigned long long;

private:
  id_type_internal id_ = reinterpret_cast<id_type_internal>(this);

public:
  bool operator==(const id_t__ &r_) const noexcept { return (id_ == r_.id_); }

  bool operator<(const id_t__ &r_) const noexcept { return (id_ < r_.id_); }
  operator id_type_internal() noexcept { return id_; }

  friend constexpr bool operator==(const id_t__ &,
                                   const id_t__::id_type_internal &) noexcept;
  friend constexpr bool operator==(const id_t__::id_type_internal &,
                                   const id_t__ &) noexcept;

  friend constexpr bool operator<(const id_t__ &,
                                  const id_t__::id_type_internal &) noexcept;
  friend constexpr bool operator<(const id_t__::id_type_internal &,
                                  const id_t__ &) noexcept;
};

constexpr bool operator==(const id_t__ &l_,
                          const id_t__::id_type_internal &id_) noexcept {
  return (l_.id_ == id_);
}
constexpr bool operator==(const id_t__::id_type_internal &id_,
                          const id_t__ &l_) noexcept {
  return (id_ == l_.id_);
}

constexpr bool operator<(const id_t__ &l_,
                         const id_t__::id_type_internal &id_) noexcept {
  return (l_.id_ < id_);
}
constexpr bool operator<(const id_t__::id_type_internal &id_,
                         const id_t__ &l_) noexcept {
  return (id_ < l_.id_);
}
} // namespace signaler::detail

#endif // SIGNALER_ID_GEN_CON_HPP