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

template <typename T, size_t queue_lenght = 8>
using queue_t =
#ifdef SIGNALER_FREE_RTOS
    free_rtos_queue_t<T, queue_lenght>;
#else
    std_queue_t<T, queue_lenght>;
#endif

} // namespace signaler::detail

#endif