/**
 ******************************************************************************
 * @file             context.hpp
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

#ifndef SIGNALER_CONTEXT_T_HPP
#define SIGNALER_CONTEXT_T_HPP

#include "detail/inter_contexts_queue/queue.hpp"
#include <type_traits>

namespace signaler {

struct icontext_t {

  using task_t = typename signaler::function_t<void(), 128>;

  template <template <typename, size_t> class queue_type, size_t SIZE = 16>
  using queue_tasks_t = queue_type<task_t, 16>;

  virtual ~icontext_t() noexcept {}
  virtual result_t<void> schedule(task_t &&task_) noexcept = 0;
};

template <size_t queue_size = 16,
          template <typename, size_t> class T = signaler::detail::queue_t>
class context_t final : public icontext_t {

  using queue_t__ = typename signaler::detail::require_queue_concept_for<
      queue_tasks_t<T, queue_size>>::queue_t__;
  queue_t__ queue_;

public:
  context_t() noexcept {}
  virtual ~context_t() noexcept {}

  result_t<void> operator()() noexcept {

    result_t<void> res_;

    for (;;) {
      result_t<task_t> q_item_ = queue_.wait_pop();

      if (q_item_)
        q_item_.value()();

      res_ = q_item_.error();
      if (!res_)
        break;
    }

    return res_;
  }

  virtual result_t<void> schedule(task_t &&task_) noexcept final override {
    return queue_.push(std::move(task_));
  }
};
} // namespace signaler

#endif // SIGNALER_CONTEXT_T_HPP