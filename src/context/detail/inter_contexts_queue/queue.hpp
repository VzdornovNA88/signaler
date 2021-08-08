/**
 ******************************************************************************
 * @file             queue.hpp
 * @brief            this is queue between two  differnt contexts
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

#ifndef SIGNALER_QUEUE_T_HPP
#define SIGNALER_QUEUE_T_HPP

#include "../../../function/function.hpp"
#include "queue_traits.hpp"


#ifdef SIGNALER_FREE_RTOS
  #include "platforms/free_rtos_queue.hpp"
#else
  #include "platforms/std_queue.hpp"
#endif

namespace signaler::detail {

using event_t =
#ifdef SIGNALER_FREE_RTOS
    typename signaler::function_t<void(), 32>;
#else
    typename signaler::function_t<void(), 128>;
#endif

template <size_t queue_lenght = 8>
using event_queue_t =
#ifdef SIGNALER_FREE_RTOS
    typename require_queue_concept_for<
        free_rtos_queue_t<event_t, queue_lenght>>::queue_t;
#else
    typename require_queue_concept_for<
        std_queue_t<event_t, queue_lenght>>::queue_t;
#endif

template <typename T, size_t queue_lenght = 8>
using queue_t =
#ifdef SIGNALER_FREE_RTOS
    typename require_queue_concept_for<
        free_rtos_queue_t<T, queue_lenght>>::queue_t;
#else
    typename require_queue_concept_for<std_queue_t<T, queue_lenght>>::queue_t;
#endif

} // namespace signaler::detail

#endif