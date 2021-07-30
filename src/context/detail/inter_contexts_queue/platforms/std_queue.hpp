/**
 ******************************************************************************
 * @file             std_queue.hpp
 * @brief
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

#ifndef SIGNALER_STD_QUEUE_T_HPP
#define SIGNALER_STD_QUEUE_T_HPP

#include <array>
#include <condition_variable>
#include <mutex>


#include "../../../../result/result.hpp"
#include "../queue_traits.hpp"

namespace signaler::detail {

template <typename T, size_t N> class std_queue_t {
public:
  using value_type = T;

private:
  using queue_storage_t = std::array<T, N>;

  queue_storage_t storage_;
  mutable std::mutex mutex_;
  std::condition_variable event_;
  typename queue_storage_t::iterator front_{storage_.begin()};
  typename queue_storage_t::iterator back_{storage_.begin()};

public:
  typename queue_traits<std_queue_t>::ret_type_for_push_t
  push(value_type &&v_) noexcept {

    using std::begin;
    using std::end;

    try {
      std::lock_guard<std::mutex> lock_(mutex_);

      queue_status_t status_ = queue_status_t::Q_READY;

      *front_ = std::move(v_);
      front_++;

      if (front_ == end(storage_))
        front_ = begin(storage_);

      if (front_ == back_) {

        status_ = queue_status_t::Q_OVERFLOW;

        back_++;
        if (back_ == end(storage_)) {
          back_ = begin(storage_);
        }
      }

      event_.notify_one();

      return {status_};
    } catch (...) {

      return {queue_status_t::Q_PUSH_LOCK_ERROR};
    }
  }

  typename queue_traits<std_queue_t>::ret_type_for_wait_pop_t
  wait_pop() noexcept {

    try {
      std::unique_lock<std::mutex> lock_(mutex_);

      event_.wait(lock_, [this] { return (front_ != back_); });

      auto res_ = back_;
      back_++;

      if (back_ == end(storage_))
        back_ = begin(storage_);

      return {std::move(*res_), queue_status_t::Q_READY};
    } catch (...) {

      return {{}, queue_status_t::Q_POP_LOCK_ERROR};
    }
  }

  typename queue_traits<std_queue_t>::ret_type_for_empty_t
  empty() const noexcept {

    try {
      std::lock_guard<std::mutex> lock_(mutex_);

      return (front_ == back_);
    } catch (...) {

      return {{}, queue_status_t::Q_IS_EMPTY_LOCK_ERROR};
    }
  }

  typename queue_traits<std_queue_t>::ret_type_for_max_size_t
  max_size() const noexcept {

    return storage_.max_size();
  }

public:
  std_queue_t(std_queue_t &&) = delete;
  std_queue_t(const std_queue_t &) = delete;
  std_queue_t &operator=(const std_queue_t &) = delete;
  std_queue_t &operator=(std_queue_t &&) = delete;

  std_queue_t() noexcept {};
};
} // namespace signaler::detail

#endif
