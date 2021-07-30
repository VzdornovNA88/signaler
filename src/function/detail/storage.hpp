/**
 ******************************************************************************
 * @file             storage.hpp
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

#ifndef SIGNALER_STORAGE_HPP
#define SIGNALER_STORAGE_HPP

#include <functional>
#include <new>
#include <type_traits>
#include <variant>

namespace signaler::detail {

template <size_t SMALL_OPT_SIZE> class storage_t final {

  static_assert(SMALL_OPT_SIZE >= 16,
                "The NTT parameter SMALL_OPT_SIZE in storage frame of "
                "function_t type shall be greater than or equal to 16 bytes !");

  struct control_t {

    using destructor_t = void (*)(void *);

    size_t cnt = 0;
    destructor_t destruct_ = nullptr;
  };
  template <typename T> struct control_block_t final : control_t {

    T payload;

    control_block_t(T &&_payload) noexcept
        : payload(std::forward<T>(_payload)) {
      this->destruct_ = payload_destructor;
    }

    control_block_t(const T &_payload) noexcept : payload(_payload) {
      this->destruct_ = payload_destructor;
    }

    static void payload_destructor(void *o) noexcept {
      static_cast<control_block_t<T> *>(o)->payload.~T();
    }
  };

  enum storage_state_t : size_t {
    INVALID = 0,
    LOCAL = 1,
    DYNAMIC = 2,
    POINTER = 3,
    POINTER_F = 4,
  };

  struct small_t {
    std::byte array[SMALL_OPT_SIZE];
  };

  using small_object_t =
      typename std::aligned_storage<sizeof(small_t), alignof(small_t)>::type;
  using big_object_t = void *;
  using ptr_object_t = std::byte *;
  using ptr_function_t = void (*)();

  std::variant<std::monostate, small_object_t, big_object_t, ptr_object_t,
               ptr_function_t>
      store;

  void destructor(void *p_store) noexcept {

    if (p_store == nullptr)
      return;

    auto cb = static_cast<control_t *>(p_store);
    auto &cnt_ref = cb->cnt;

    if (cnt_ref == 0) {

      cb->destruct_(p_store);
      operator delete(p_store);
      store = std::monostate{};
    } else {
      --cnt_ref;
      store = std::monostate{};
    }
  }

public:
  storage_t() noexcept = default;
  storage_t(std::nullptr_t const) noexcept : storage_t() {}
  ~storage_t() noexcept {
    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);
  }

  storage_t(storage_t const &s) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    store = s.store;

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      if (*p_store != nullptr)
        ++static_cast<control_t *>(*p_store)->cnt;
  }

  storage_t(storage_t &&s) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    store = std::move(s.store);

    if (auto p_store = std::get_if<DYNAMIC>(&s.store)) {
      *p_store = nullptr;
      s.store = std::monostate{};
    }
  }

  storage_t &operator=(storage_t const &s) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    store = s.store;

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      if (*p_store != nullptr)
        ++static_cast<control_t *>(*p_store)->cnt;

    return *this;
  }

  storage_t &operator=(storage_t &&s) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    store = std::move(s.store);

    if (auto p_store = std::get_if<DYNAMIC>(&s.store)) {
      *p_store = nullptr;
      s.store = std::monostate{};
    }

    return *this;
  }

  storage_t &operator=(std::nullptr_t const) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    store = std::monostate{};

    return *this;
  }

  storage_t &operator=(int const) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    store = std::monostate{};

    return *this;
  }

  template <typename T> storage_t(T f) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    using functor_t = typename std::decay<T>::type;

    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      store = ptr_function_t();
      auto p_store = std::get_if<POINTER_F>(&store);
      new (p_store) functor_t(f);
    } else if constexpr (std::is_pointer_v<T>) {
      store = ptr_object_t();
      auto p_store = std::get_if<POINTER>(&store);
      new (p_store) T(f);
    } else if constexpr (sizeof(functor_t) > sizeof(small_t)) {

      const std::nothrow_t noexcept_v;
      auto p_store = operator new(sizeof(control_block_t<functor_t>),
                                  noexcept_v);

      if (p_store == nullptr)
        store = std::monostate{};
      else {
        new (p_store) control_block_t<functor_t>(std::forward<T>(
            f)); /* ??? constructor of functor_t can throws any exception */
        store = p_store;
      }
    } else {
      store = small_object_t();
      auto p_store = std::get_if<LOCAL>(&store);
      new (p_store) functor_t(std::forward<T>(f));
    }
  }

  template <typename T> storage_t &operator=(T f) noexcept {

    if (auto p_store = std::get_if<DYNAMIC>(&store))
      destructor(*p_store);

    using functor_t = typename std::decay<T>::type;

    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      store = ptr_function_t();
      auto p_store = std::get_if<POINTER_F>(&store);
      new (p_store) functor_t(f);
    } else if constexpr (std::is_pointer_v<T>) {
      store = ptr_object_t();
      auto p_store = std::get_if<POINTER>(&store);
      new (p_store) T(f);
    } else if constexpr (sizeof(functor_t) > sizeof(small_t)) {

      const std::nothrow_t noexcept_v;
      auto p_store = operator new(sizeof(control_block_t<functor_t>),
                                  noexcept_v);

      if (p_store == nullptr)
        store = std::monostate{};
      else {
        new (p_store) control_block_t<functor_t>(std::forward<T>(
            f)); /* ??? constructor of functor_t can throws any exception */
        store = p_store;
      }
    } else {
      store = small_object_t();
      auto p_store = std::get_if<LOCAL>(&store);
      new (p_store) functor_t(std::forward<T>(f));
    }

    return *this;
  }

  template <typename T> [[nodiscard]] constexpr auto get() noexcept {

    using object_t = typename std::decay<T>::type;

    if constexpr (std::is_function_v<std::remove_pointer_t<T>>)
      return reinterpret_cast<T>(*std::get_if<POINTER_F>(&store));
    else if constexpr (std::is_pointer_v<T>) {
      return std::launder(reinterpret_cast<T>(*std::get_if<POINTER>(&store)));
    } else if constexpr (sizeof(object_t) > sizeof(small_t)) {
      return std::launder(
          reinterpret_cast<T *>(&static_cast<control_block_t<object_t> *>(
                                     *std::get_if<DYNAMIC>(&store))
                                     ->payload));
    } else {
      return std::launder(reinterpret_cast<T *>(std::get_if<LOCAL>(&store)));
    }
  }

  bool operator==(storage_t const &r) const noexcept {

    if (store.index() == INVALID && r.store.index() == INVALID)
      return true;
    else if (store.index() == POINTER_F && r.store.index() == POINTER_F)
      return (*std::get_if<POINTER_F>(&store) ==
              *std::get_if<POINTER_F>(&r.store));
    else if (store.index() == POINTER && r.store.index() == POINTER)
      return (*std::get_if<POINTER>(&store) == *std::get_if<POINTER>(&r.store));
    else
      return false;
  }

  bool operator==(std::nullptr_t const) const noexcept {

    return store.index() == INVALID;
  }

  bool operator!=(std::nullptr_t const) const noexcept {

    return store.index() != INVALID;
  }
};

} // namespace signaler::detail

#endif // SIGNALER_STORAGE_HPP