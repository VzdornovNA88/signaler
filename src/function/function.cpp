/**
 ******************************************************************************
 * @file             function.cpp
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

#include "function.hpp"

namespace {

struct function_status_category_t : std::error_category {
  const char *name() const noexcept override;
  std::string message(int ev) const override;
};

const char *function_status_category_t::name() const noexcept {
  return "ERRORS in function_t of signaler library";
}

std::string function_status_category_t::message(int s_) const {
  switch (static_cast<signaler::function_status_t>(s_)) {
  case signaler::function_status_t::F_CALL_SUCCESS:
    return "INFORMATION in signaler - function call is successful !";

  case signaler::function_status_t::F_CALL_FAILED_FUNCTION_IS_NOT_INITIALIZED:
    return "ERROR in signaler - function call is failed, function is not "
           "initialized !";

  default:
    return "ERROR in signaler - FATAL ERROR (unrecognized status for "
           "function_t)";
  }
}

const function_status_category_t category__{};
} // namespace

namespace signaler {
std::error_code make_error_code(signaler::function_status_t s_) {
  return {static_cast<int>(s_), category__};
}
} // namespace signaler