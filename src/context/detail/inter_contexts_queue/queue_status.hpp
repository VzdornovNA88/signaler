/**
 ******************************************************************************
 * @file             queue_status.hpp
 * @brief            This is a category of errors,warnings,info massages
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

#ifndef SIGNALER_QUEUE_STATUS_HPP
#define SIGNALER_QUEUE_STATUS_HPP

#include <system_error>

namespace signaler::detail {
enum class queue_status_t : unsigned char {

  Q_READY = 0,
  Q_OVERFLOW = 1,
  Q_POP_LOCK_ERROR = 2,
  Q_PUSH_LOCK_ERROR = 3,
  Q_IS_EMPTY_LOCK_ERROR = 4,
};

std::error_code make_error_code(queue_status_t);
} // namespace signaler::detail

namespace std {
template <>
struct is_error_code_enum<signaler::detail::queue_status_t> : true_type {};
} // namespace std

#endif // SIGNALER_QUEUE_STATUS_HPP