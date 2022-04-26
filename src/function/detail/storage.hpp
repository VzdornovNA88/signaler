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

#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <new>
#include <type_traits>
#include <utility>
#include <variant>

#include "polymorphic_allocator_noexcept.hpp"

namespace signaler::detail {

enum class atomicity_policy_t : unsigned char { NON_ATOMIC = 0, ATOMIC = 1 };

template <
    size_t SMALL_OPT_SIZE,
    atomicity_policy_t ATOMICITY_POLICY /*= atomicity_policy_t::NON_ATOMIC*/>
class storage_t__ final {

  static_assert(SMALL_OPT_SIZE >= 16,
                "[signaler] : The NTT parameter SMALL_OPT_SIZE for storage frame of shall "
                "be greater than or equal to 16 bytes !");

  template <typename T>
  static auto &
  allocator_instance(memmory_resource_noexcept_t *res = nullptr) noexcept {

    // using allocator_t__ = std::pmr::polymorphic_allocator<T>;
    static polymorphic_allocator_noexcept_t<T> alloc_{res};
    return alloc_;
  }
  struct control_t__ {
    struct small_t__ {
      std::byte array[SMALL_OPT_SIZE];
    };
    using optimized_aligned_storage_t__ =
        typename std::aligned_storage<sizeof(small_t__),
                                      alignof(small_t__)>::type;

    enum class operation_t__ : unsigned char {
      CSTR = 0,
      COPY = 1,
      MOVE = 2,
      DSTR = 3,
      COMP = 4,
    };

    [[maybe_unused]] static bool vtable_default(operation_t__, void *,
                                                [[maybe_unused]] void *) {
      return true;
    };

    template <typename, typename = std::void_t<>>
    struct is_comparable_t__ : std::false_type {};
    template <typename T>
    struct is_comparable_t__<T, ::std::enable_if_t<::std::is_same<
                                    bool, decltype(std::declval<T>().operator==(
                                              std::declval<T>()))>::value>>
        : std::true_type {};

    using operation_dispatch_t__ = bool (*)(operation_t__, void *,
                                            [[maybe_unused]] void *);

    operation_dispatch_t__ operation_dispatch_ = vtable_default;
    optimized_aligned_storage_t__ payload_;
  };

  struct big_object_t__ final : control_t__ {

    using underline_counter_t__ = size_t;
    using counter_t__ =
        std::conditional_t<atomicity_policy_t::ATOMIC == ATOMICITY_POLICY,
                           std::atomic<underline_counter_t__>,
                           underline_counter_t__>;
    struct big_aligned_storage_t__ {
      counter_t__ cnt_ = 1;
      void *object_ = nullptr;
    };

    template <typename T = void *>
    [[maybe_unused]] static bool vtable(typename control_t__::operation_t__ op,
                                        void *to,
                                        [[maybe_unused]] void *from = nullptr) {

      using allocator_traits_t__ =
          std::allocator_traits<polymorphic_allocator_noexcept_t<T>>;

      if constexpr (std::is_same_v<T, void *>)
        return false;
      else {
        switch (op) {
        case control_t__::operation_t__::CSTR: {
          new (to) big_aligned_storage_t__;
          auto big_obj_ =
              std::launder(reinterpret_cast<big_aligned_storage_t__ *>(to));
          big_obj_->object_ =
              allocator_traits_t__::allocate(allocator_instance<T>(), 1);
          if (big_obj_->object_)
            allocator_traits_t__::construct(allocator_instance<T>(),
                                            static_cast<T*>(big_obj_->object_),
                                            std::move(*static_cast<T*>(from)));
        } break;
        case control_t__::operation_t__::COPY: {
          new (to) big_aligned_storage_t__;
          auto to_ =
              std::launder(reinterpret_cast<big_aligned_storage_t__ *>(to));
          auto from_ =
              std::launder(reinterpret_cast<big_aligned_storage_t__ *>(from));
          to_->object_ = from_->object_;
          to_->cnt_ = from_->cnt_++;
        } break;
        case control_t__::operation_t__::MOVE: {
          new (to) big_aligned_storage_t__;
          auto to_ =
              std::launder(reinterpret_cast<big_aligned_storage_t__ *>(to));
          auto from_ =
              std::launder(reinterpret_cast<big_aligned_storage_t__ *>(from));
          to_->object_ = from_->object_;
          to_->cnt_ = static_cast<underline_counter_t__>(from_->cnt_);
        } break;
        case control_t__::operation_t__::DSTR: {
          static_assert(std::is_nothrow_destructible_v<T>,
                        "[signaler] : Type 'T' in big_object_t__ of storage_t__ doesn't "
                        "satisfy for is_nothrow_destructiable_v requirement");
          auto from_ =
              std::launder(reinterpret_cast<big_aligned_storage_t__ *>(from));
          if (--from_->cnt_ == 0 && from_->object_) {
            auto o_ = static_cast<T*>(from_->object_);
            allocator_traits_t__::destroy(allocator_instance<T>(), o_);
            allocator_traits_t__::deallocate(allocator_instance<T>(), o_, 1);
          }
          from_->~big_aligned_storage_t__();

        } break;
        case control_t__::operation_t__::COMP: {
          if constexpr (not control_t__::template is_comparable_t__<T>::value)
            return false;
          else {
            auto big_obj_this_ =
                std::launder(
                    reinterpret_cast<const big_aligned_storage_t__ *>(to))
                    ->object_;
            auto big_obj_that_ =
                std::launder(
                    reinterpret_cast<const big_aligned_storage_t__ *>(from))
                    ->object_;
            return !(nullptr == big_obj_this_ || nullptr == big_obj_that_) &&
                   *static_cast<T *>(big_obj_this_) ==
                       *static_cast<T *>(big_obj_that_);
          }
        } break;
        }
      }
      return true;
    }

    template <typename T>
    big_object_t__(T object) noexcept(noexcept(std::move(std::declval<T>()))) {
      control_t__::operation_dispatch_ = vtable<T>;
      control_t__::operation_dispatch_(control_t__::operation_t__::CSTR,
                                       &this->payload_, &object);
    }

    ~big_object_t__() noexcept {
      control_t__::operation_dispatch_(control_t__::operation_t__::DSTR,
                                       nullptr, &this->payload_);
    }

    big_object_t__(big_object_t__ const &s) noexcept {
      control_t__::operation_dispatch_ = s.operation_dispatch_;
      control_t__::operation_dispatch_(
          control_t__::operation_t__::COPY, &this->payload_,
          &const_cast<big_object_t__ *>(&s)->payload_);
    }

    big_object_t__(big_object_t__ &&s) noexcept {
      control_t__::operation_dispatch_ = s.operation_dispatch_;
      control_t__::operation_dispatch_(control_t__::operation_t__::MOVE,
                                       &this->payload_, &s.payload_);
      s.operation_dispatch_ = control_t__::vtable_default;
    }

    big_object_t__ &operator=(big_object_t__ const &s) noexcept {
      this->~big_object_t__();
      ::new (this) big_object_t__(s);
      return *this;
    }

    big_object_t__ &operator=(big_object_t__ &&s) noexcept {
      this->~big_object_t__();
      ::new (this) big_object_t__(std::move(s));
      return *this;
    }

    bool operator==(big_object_t__ const &s) const noexcept {
      return control_t__::operation_dispatch_ == s.operation_dispatch_ &&
             control_t__::operation_dispatch_(
                 control_t__::operation_t__::COMP,
                 &const_cast<big_object_t__ *>(this)->payload_,
                 &const_cast<big_object_t__ *>(&s)->payload_);
    }

    bool is_valid() const noexcept {
      return std::launder(reinterpret_cast<const big_aligned_storage_t__ *>(
                              &this->payload_))
                 ->object_ != nullptr;
    }
  };

  struct small_object_t__ final : control_t__ {

    template <typename T = void *>
    [[maybe_unused]] static bool vtable(typename control_t__::operation_t__ op,
                                        void *to,
                                        [[maybe_unused]] void *from = nullptr) {

      if constexpr (std::is_same_v<T, void *>)
        return false;
      else {

        if (nullptr == to)
          return false;

        auto to_ = static_cast<T *>(to);

        switch (op) {
        case control_t__::operation_t__::CSTR: {
          if (nullptr == from)
            return false;
          static_assert(
              std::is_nothrow_move_constructible_v<T>,
              "[signaler] : Type 'T' in small_type_t__ of storage_t__ doesn't "
              "satisfy for is_nothrow_move_constructible requirement");
          new (to) T(std::move(*static_cast<T *>(from)));
        } break;
        case control_t__::operation_t__::COPY: {
          if (nullptr == from)
            return false;
          static_assert(
              std::is_nothrow_copy_constructible_v<T>,
              "[signaler] : Type 'T' in small_type_t__ of storage_t__ doesn't "
              "satisfy for is_nothrow_copy_constructible requirement");
          ::new (to_) T(*static_cast<T *>(from));
        } break;
        case control_t__::operation_t__::MOVE: {
          if (nullptr == from)
            return false;
          static_assert(
              std::is_nothrow_move_constructible_v<T>,
              "[signaler] : Type 'T' in small_type_t__ of storage_t__ doesn't "
              "satisfy for is_nothrow_move_constructible requirement");
          new (to_) T(std::move(*static_cast<T *>(from)));
        } break;
        case control_t__::operation_t__::DSTR: {
          static_assert(std::is_nothrow_destructible_v<T>,
                        "[signaler] : Type 'T' in small_type_t__ of storage_t__ doesn't "
                        "satisfy for is_nothrow_destructiable_v requirement");
          to_->~T();
        } break;
        case control_t__::operation_t__::COMP: {
          if constexpr (not control_t__::template is_comparable_t__<T>::value) {
            return false;
          } else {
            return !(nullptr == to || nullptr == from) &&
                   *static_cast<T *>(to) == *static_cast<T *>(from);
          }
        } break;
        }
        return true;
      }
    }

    template <typename T> small_object_t__(T object) noexcept {
      control_t__::operation_dispatch_ = vtable<T>;
      control_t__::operation_dispatch_(control_t__::operation_t__::CSTR,
                                       &this->payload_, &object);
    }

    ~small_object_t__() noexcept {
      control_t__::operation_dispatch_(control_t__::operation_t__::DSTR,
                                       &this->payload_, nullptr);
    }

    small_object_t__(small_object_t__ const &s) noexcept {
      control_t__::operation_dispatch_ = s.operation_dispatch_;
      control_t__::operation_dispatch_(
          control_t__::operation_t__::COPY, &this->payload_,
          &const_cast<small_object_t__ *>(&s)->payload_);
    }

    small_object_t__(small_object_t__ &&s) noexcept {
      this->operation_dispatch_ = s.operation_dispatch_;
      this->operation_dispatch_(control_t__::operation_t__::MOVE,
                                &this->payload_, &s.payload_);
      s.operation_dispatch_ = control_t__::vtable_default;
    }

    small_object_t__ &operator=(small_object_t__ const &s) noexcept {
      this->~small_object_t__();
      ::new (this) small_object_t__(s);
      return *this;
    }

    small_object_t__ &operator=(small_object_t__ &&s) noexcept {
      this->~small_object_t__();
      ::new (this) small_object_t__(std::move(s));
      return *this;
    }

    bool operator==(small_object_t__ const &s) const noexcept {
      return (control_t__::operation_dispatch_ == s.operation_dispatch_ &&
              control_t__::operation_dispatch_(
                  control_t__::operation_t__::COMP,
                  &const_cast<small_object_t__ *>(this)->payload_,
                  &const_cast<small_object_t__ *>(&s)->payload_));
    }
  };

  using ptr_object_t__ = std::byte *;
  using ptr_function_t__ = void (*)();
  using internal_storage_t__ =
      std::variant<std::monostate, small_object_t__, big_object_t__,
                   ptr_object_t__, ptr_function_t__>;
  internal_storage_t__ store;

  enum storage_state_t__ : size_t {
    INVALID = 0,
    LOCAL = 1,
    DYNAMIC = 2,
    POINTER = 3,
    POINTER_F = 4,
  };

public:
  storage_t__() noexcept = default;
  ~storage_t__() noexcept = default;

  storage_t__(storage_t__ const &s) noexcept : store(s.store){};
  storage_t__ &operator=(storage_t__ const &s) noexcept {
    store = s.store;
    return *this;
  };

  storage_t__(storage_t__ &&s) noexcept = default;
  storage_t__ &operator=(storage_t__ &&s) noexcept = default;

  storage_t__(std::nullptr_t const) noexcept : storage_t__() {}

  storage_t__ &operator=(std::nullptr_t const) noexcept {
    store = std::monostate{};
    return *this;
  }
  storage_t__ &operator=(int const) noexcept {
    store = std::monostate{};
    return *this;
  }

  template <typename T>
  storage_t__(T f, memmory_resource_noexcept_t *res = nullptr) noexcept(
      noexcept(std::move(std::declval<T>()))) {
    using functor_t__ = typename std::decay<T>::type;

    allocator_instance<T>(res);

    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      store = ptr_function_t__(f);
    } else if constexpr (std::is_pointer_v<T>) {
      store = ptr_object_t__(f);
    } else if constexpr (sizeof(functor_t__) >
                         sizeof(small_object_t__::payload_)) {
      store = std::monostate{};
      big_object_t__ obj_(std::forward<T>(f));
      if (obj_.is_valid())
        store = std::move(obj_);
    } else {
      store = small_object_t__(std::forward<T>(f));
    }
  }

  template <typename T>
  storage_t__ &operator=(T f) noexcept(noexcept(std::move(std::declval<T>()))) {
    using functor_t__ = typename std::decay<T>::type;
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>) {
      store = ptr_function_t__(f);
    } else if constexpr (std::is_pointer_v<T>) {
      store = ptr_object_t__(f);
    } else if constexpr (sizeof(functor_t__) >
                         sizeof(small_object_t__::payload_)) {
      big_object_t__ obj_(std::forward<T>(f));
      store = obj_.is_valid() ? std::move(obj_) : std::monostate{};
    } else {
      store = small_object_t__(std::forward<T>(f));
    }
    return *this;
  }

  template <typename T> [[nodiscard]] auto get() &noexcept {
    using object_t__ = typename std::decay<T>::type;
    if constexpr (std::is_function_v<std::remove_pointer_t<T>>)
      return reinterpret_cast<T>(*std::get_if<POINTER_F>(&store));
    else if constexpr (std::is_pointer_v<T>) {
      return std::launder(reinterpret_cast<T>(*std::get_if<POINTER>(&store)));
    } else if constexpr (sizeof(object_t__) >
                         sizeof(small_object_t__::payload_)) {
      return std::launder(reinterpret_cast<T *>(
          reinterpret_cast<typename big_object_t__::big_aligned_storage_t__ *>(
              &static_cast<big_object_t__ *>(std::get_if<DYNAMIC>(&store))
                   ->payload_)
              ->object_));
    } else {
      return std::launder(reinterpret_cast<T *>(
          &static_cast<small_object_t__ *>(std::get_if<LOCAL>(&store))
               ->payload_));
    }
  }

  template <typename T> [[nodiscard]] auto get() const &&noexcept = delete;
  template <typename T> [[nodiscard]] auto get() &&noexcept = delete;

  bool operator==(storage_t__ const &r) const noexcept {
    return this->store == r.store;
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