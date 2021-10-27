/**
 ******************************************************************************
 * @file             object.hpp
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

#ifndef SIGNALER_OBJECT_T_HPP
#define SIGNALER_OBJECT_T_HPP

#include "context.hpp"
#include <memory>

namespace signaler {

struct iobject_t {
  virtual icontext_t *context() const& noexcept = 0;
};

template <auto ctx_> struct object_t : iobject_t {

  static_assert(
      std::is_pointer<decltype(ctx_)>::value &&
          std::is_base_of<icontext_t,
                          std::remove_pointer_t<decltype(ctx_)>>::value,
      "In the declared object_t <T> context, the template parameter T must be "
      "a non-type template parameter that must be convertible to icontext_t* "
      "!");

  icontext_t *context() const& noexcept final override { return ctx_; }
  icontext_t *context() const&& noexcept = delete;
  icontext_t *context() && noexcept = delete;
};

} // namespace signaler

#endif // SIGNALER_OBJECT_T_HPP