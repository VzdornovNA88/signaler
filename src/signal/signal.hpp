/**
 ******************************************************************************
 * @file             signal.hpp
 * @brief            Simple signal-object pattern
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

#ifndef SIGNALER_SIGNAL_HPP
#define SIGNALER_SIGNAL_HPP

#include "../context/detail/inter_contexts_queue/queue.hpp"
#include "../context/object.hpp"
#include "../function/function.hpp"
#include "../result/result.hpp"
#include "detail/id_connection_generator.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

namespace signaler {

enum class signal_status_t : unsigned char {

  S_READY = 0,
  S_CONNECTION_FAILED = 1,
  S_SLOT_QUEUE_OVERFLOW = 2,
  S_RESULT_QUEUE_OVERFLOW = 3,
  S_RESULT_QUEUE_ERROR_LOCK = 4,
  S_SLOT_QUEUE_ERROR_LOCK = 5,
};
}

namespace std {
template <> struct is_error_code_enum<signaler::signal_status_t> : true_type {};
} // namespace std

namespace signaler {

std::error_code make_error_code(signal_status_t);

namespace detail {

template <typename T> struct signal_base_t__ {
  template <typename> static constexpr bool always_false_v = false;
  static_assert(always_false_v<T>,
                "[signaler] : signature of callable object does not satisfy "
                "'R(A...) CONST/VOLATILE/REF/NOEXCEPT'");
};

#define DEFINE_BASE_SIGNAL_VARIANT(CONST, VOLATILE, NOEXCEPT, REF)             \
  template <template <typename> class DERIVED, typename R, typename... A>      \
  struct signal_base_t__<DERIVED<R(A...) CONST VOLATILE REF NOEXCEPT>> {       \
  public:                                                                      \
    void operator()(A... args) noexcept {                                      \
      using derived_t__ = DERIVED<R(A...) CONST VOLATILE REF NOEXCEPT>;        \
      return static_cast<derived_t__ *>(this)->notify(                         \
          std::forward<A>(args)...);                                           \
    }                                                                          \
  };

DEFINE_BASE_SIGNAL_VARIANT(const, volatile, noexcept, &)

DEFINE_BASE_SIGNAL_VARIANT(const, volatile, noexcept, )
DEFINE_BASE_SIGNAL_VARIANT(const, volatile, , &)
DEFINE_BASE_SIGNAL_VARIANT(const, , noexcept, &)
DEFINE_BASE_SIGNAL_VARIANT(, volatile, noexcept, &)

DEFINE_BASE_SIGNAL_VARIANT(const, , , )
DEFINE_BASE_SIGNAL_VARIANT(const, volatile, , )
DEFINE_BASE_SIGNAL_VARIANT(const, , noexcept, )
DEFINE_BASE_SIGNAL_VARIANT(const, , , &)

DEFINE_BASE_SIGNAL_VARIANT(, volatile, , )
DEFINE_BASE_SIGNAL_VARIANT(, volatile, noexcept, )
DEFINE_BASE_SIGNAL_VARIANT(, volatile, , &)

DEFINE_BASE_SIGNAL_VARIANT(, , noexcept, )
DEFINE_BASE_SIGNAL_VARIANT(, , noexcept, &)

DEFINE_BASE_SIGNAL_VARIANT(, , , &)

DEFINE_BASE_SIGNAL_VARIANT(, , , )

DEFINE_BASE_SIGNAL_VARIANT(const, volatile, noexcept, &&)

DEFINE_BASE_SIGNAL_VARIANT(const, volatile, , &&)
DEFINE_BASE_SIGNAL_VARIANT(const, , noexcept, &&)
DEFINE_BASE_SIGNAL_VARIANT(, volatile, noexcept, &&)

DEFINE_BASE_SIGNAL_VARIANT(const, , , &&)

DEFINE_BASE_SIGNAL_VARIANT(, volatile, , &&)

DEFINE_BASE_SIGNAL_VARIANT(, , noexcept, &&)

DEFINE_BASE_SIGNAL_VARIANT(, , , &&)

template <typename SIGNATURE>
class signal_t__ final : public signal_base_t__<signal_t__<SIGNATURE>> {
public:
  using slot_t = function_t<SIGNATURE, 16, detail::atomicity_policy_t::ATOMIC>;
  class connection_t;

private:
  using base_t__ = signal_base_t__<signal_t__<SIGNATURE>>;
  friend struct signal_base_t__<signal_t__<SIGNATURE>>;

  using result_type_t__ = typename slot_t::traits_t::result_type_t__;

  template <typename T,
            typename T_ = std::conditional_t<std::is_rvalue_reference_v<T>,
                                             std::remove_reference_t<T>, T>>
  struct arg_t__ {
    T_ arg;
    operator T_() noexcept {
      if constexpr (!std::is_lvalue_reference_v<T>)
        return std::move(arg);
      else
        return arg;
    }
  };

  template <typename T> class future_base_t__ {
    friend class signal_t__<SIGNATURE>;

    result_t<T> status_{std::error_code{signal_status_t::S_CONNECTION_FAILED}};
    bool is_connected{false};

  protected:
    virtual void set(result_t<T> v_) noexcept { status_ = v_; };

  public:
    virtual ~future_base_t__() noexcept {}

    [[nodiscard]] virtual const result_t<T> get() noexcept { return status_; }
  };
  class private_connection_t__ {

    detail::id_t__ id_;
    icontext_t *ctx_ = nullptr;
    slot_t slot_;

    using result_ptr_t = std::shared_ptr<future_base_t__<result_type_t__>>;
    mutable result_ptr_t result_;

    template <typename T> class future_t__ final : public future_base_t__<T> {

      friend class signal_t__<SIGNATURE>;

      using queue_t__ = detail::queue_t<T>;
      queue_t__ results_;

    protected:
      virtual void set(result_t<T> v_) noexcept final override {

        auto res_ = results_.push(v_.value());

        if (res_.error() == detail::queue_status_t::Q_OVERFLOW)
          future_base_t__<T>::status_ =
              std::error_code{signal_status_t::S_RESULT_QUEUE_OVERFLOW};
        else if (res_.error() == detail::queue_status_t::Q_PUSH_LOCK_ERROR)
          future_base_t__<T>::status_ =
              std::error_code{signal_status_t::S_RESULT_QUEUE_ERROR_LOCK};
      }

    public:
      future_t__() noexcept = default;
      virtual ~future_t__() noexcept = default;

      [[nodiscard]] virtual const result_t<T> get() noexcept final override {

        auto res_ = results_.wait_pop();

        if (res_.error() == detail::queue_status_t::Q_POP_LOCK_ERROR)
          return std::error_code{signal_status_t::S_RESULT_QUEUE_ERROR_LOCK};

        future_base_t__<T>::status_ = res_.value();

        return future_base_t__<T>::status_;
      }
    };

  public:
    private_connection_t__(icontext_t *c_, slot_t &&_s) /*noexcept*/
        : ctx_(c_), slot_(std::move(_s)) {
      if constexpr (std::is_same_v<result_type_t__, void>) {
        result_ = std::make_shared<future_base_t__<result_type_t__>>();
      } else if constexpr (!std::is_same_v<result_type_t__, void>) {

        static_assert(
            std::is_nothrow_default_constructible<result_type_t__>::value,
            "The type T does not satisfy "
            "is_nothrow_default_constructible as result type of signal");
        static_assert(
            std::is_nothrow_copy_constructible<result_type_t__>::value,
            "The type T does not satisfy "
            "is_nothrow_copy_constructible as result type of signal");
        static_assert(
            std::is_nothrow_move_constructible<result_type_t__>::value,
            "The type T does not satisfy "
            "is_nothrow_move_constructible as result type of signal");
        static_assert(std::is_nothrow_copy_assignable<result_type_t__>::value,
                      "The type T does not satisfy "
                      "is_nothrow_copy_assignable as result type of signal");
        static_assert(std::is_nothrow_move_assignable<result_type_t__>::value,
                      "The type T does not satisfy "
                      "is_nothrow_move_assignable as result type of signal");
        static_assert(std::is_nothrow_destructible<result_type_t__>::value,
                      "The type T does not satisfy "
                      "is_nothrow_destructible as result type of signal");

        if (!c_)
          result_ = std::make_shared<future_base_t__<result_type_t__>>();
        else
          result_ = std::make_shared<future_t__<result_type_t__>>();
      }

      result_->is_connected = true;
      /*result_->status_ = signal_status_t::S_READY;*/
    }

  public:
    private_connection_t__() = delete;

    friend class signal_t__<SIGNATURE>;
    friend class signal_t__<SIGNATURE>::connection_t;

    bool operator<(private_connection_t__ const &r) const noexcept {

      return (id_ < r.id_);
    }

    bool operator==(private_connection_t__ const &r) const noexcept {

      return (id_ == r.id_);
    }

    bool operator<(connection_t const &r) const noexcept {

      return (id_ < r.id_);
    }

    bool operator==(connection_t const &r) const noexcept {

      return (id_ == r.id_);
    }

    detail::id_t__::id_type_internal id() const noexcept { return id_; }
  };

  using container_connections_t__ = std::vector<private_connection_t__>;

public:
  class connection_t {
    detail::id_t__::id_type_internal id_ = 0;
    std::weak_ptr<future_base_t__<result_type_t__>> future_;

    friend class signal_t__<SIGNATURE>;

    connection_t(/*const*/ private_connection_t__ &con_) noexcept
        : id_(con_.id_), future_(con_.result_) {}

  protected:
    connection_t(detail::id_t__ id_) noexcept : id_(id_) {}

  public:
    connection_t() noexcept = default;
    connection_t(connection_t &&con_) noexcept = default;
    connection_t(const connection_t &con_) noexcept = delete;
    connection_t &operator=(connection_t &&other) noexcept = default;
    connection_t &operator=(const connection_t &other) noexcept = delete;

    ~connection_t() { disconnect(); }

    [[nodiscard]] auto get_result() const noexcept
        -> result_t<result_type_t__> {

      if (auto p_future_ = future_.lock(); p_future_ && p_future_->is_connected)
        return p_future_->get();
      else
        return std::error_code{signal_status_t::S_CONNECTION_FAILED};
    }

    bool is_connected() const noexcept {
      if (auto p_future_ = future_.lock())
        return p_future_->is_connected;
      else
        return false;
    }

    void disconnect() noexcept {
      if (auto p_future_ = future_.lock()) {
        p_future_->is_connected = false;
      }
    }

    bool operator<(connection_t const &r) const noexcept {

      return (id_ < r.id_);
    }

    bool operator==(connection_t const &r) const noexcept {

      return (id_ == r.id_);
    }

    bool operator!=(connection_t const &r) const noexcept {

      return !(id_ == r.id_);
    }
  };

  signal_t__() noexcept = default;

  signal_t__(signal_t__ &&other) noexcept {

    connections_.swap(other.connections_);
    other.connections_.clear();
  }

  ~signal_t__() noexcept { disconnect(); }

  signal_t__ &operator=(signal_t__ &&other) noexcept {

    disconnect();
    connections_.swap(other.connections_);
    other.connections_.clear();
    return *this;
  }

  template <typename T,
            typename slot_t::traits_t::template ptr_method_of__<T> m>
  [[nodiscard]] connection_t connect(T *o) & {

    auto slot_ = slot_t::template bind<T, m>(o);

    auto it = std::find_if(connections_.begin(), connections_.end(),
                           [&slot_](const auto &connection_) {
                             return connection_.slot_ == slot_;
                           });

    if (it == connections_.end()) {

      icontext_t *ctx_ = nullptr;
      if constexpr (std::is_base_of<iobject_t, T>::value)
        ctx_ = o->context();

      return connections_.emplace_back(ctx_, std::move(slot_));
    } else
      return *it;
  }

  template <typename T,
            typename slot_t::traits_t::template ptr_method_of__<T> m>
  [[nodiscard]] connection_t connect(T *o) && = delete;

  template <typename T,
            typename slot_t::traits_t::template ptr_method_of__<T> m>
  void disconnect(T *o) & {

    auto _disconnect_slot = slot_t::template bind<T, m>(o);

    auto it = std::find_if(connections_.begin(), connections_.end(),
                           [&_disconnect_slot](const auto &_connection) {
                             return _connection.slot_ == _disconnect_slot;
                           });

    if (it != connections_.end())
      connections_.erase(it);
  }

  template <typename T,
            typename slot_t::traits_t::template ptr_method_of__<T> m>
  void disconnect(T *o) && = delete;

  template <typename slot_t::traits_t::ptr_function_t__ f>
  [[nodiscard]] connection_t connect() & {

    auto slot_ = slot_t::template bind<f>();

    auto it = std::find_if(connections_.begin(), connections_.end(),
                           [&slot_](const auto &_connection) {
                             return _connection.slot_ == slot_;
                           });

    if (it == connections_.end()) {
      return connections_.emplace_back(nullptr, std::move(slot_));
    } else
      return *it;
  }

  template <typename slot_t::traits_t::ptr_function_t__>
  [[nodiscard]] connection_t connect() && = delete;

  template <typename slot_t::traits_t::ptr_function_t__ f> void disconnect() & {

    auto _disconnect_slot = slot_t::template bind<f>();

    auto it = std::find_if(connections_.begin(), connections_.end(),
                           [&_disconnect_slot](const auto &_connection) {
                             return _connection.slot_ == _disconnect_slot;
                           });

    if (it != connections_.end())
      connections_.erase(it);
  }

  template <typename slot_t::traits_t::ptr_function_t__>
  void disconnect() && = delete;

  template <typename T> [[nodiscard]] connection_t connect(T &&o) & {

    using raw_of_T_t__ = std::remove_reference_t<std::remove_pointer_t<T>>;
    slot_t slot_;
    if constexpr ((std::is_pointer_v<T> || std::is_rvalue_reference_v<T>)&&(
                      slot_t::traits_t::template is_function_object_of__<
                          raw_of_T_t__>::value)) {
      slot_ = slot_t::template bind<raw_of_T_t__, &raw_of_T_t__::operator()>(o);
    } else {
      slot_ = std::forward<T>(o);
    }

    icontext_t *ctx_ = nullptr;

    if constexpr ((std::is_reference_v<T> ||
                   std::is_rvalue_reference_v<T>)&&std::
                      is_base_of<iobject_t, std::remove_reference_t<T>>::value)
      ctx_ = o.context();
    else if constexpr (std::is_pointer_v<T> &&
                       std::is_base_of<iobject_t,
                                       std::remove_pointer_t<T>>::value)
      ctx_ = o->context();

    return connections_.emplace_back(ctx_, std::move(slot_));
  }

  template <typename T> [[nodiscard]] connection_t connect(T &&o) && = delete;

  [[nodiscard]] connection_t connect(signal_t__ &signal) & {
    static_assert(std::is_same<result_type_t__, void>::value,
                  "Return value of signal must be only 'void' type");

    return connect<base_t__, &base_t__::operator()>(&signal);
  }

  [[nodiscard]] connection_t connect(signal_t__ &signal) && = delete;

  template <typename T, typename Dependent_context_t__ = result_type_t__>
  [[nodiscard]] connection_t connect([[maybe_unused]] T *o,
                                     signal_t__ &signal) & {
    static_assert(std::is_same<result_type_t__, void>::value,
                  "Return value of signal must be only 'void' type !");

    auto slot_ =
        slot_t::template bind<base_t__, &base_t__::operator()>(&signal);

    auto it = std::find_if(connections_.begin(), connections_.end(),
                           [&slot_](const auto &_connection) {
                             return _connection.slot_ == slot_;
                           });

    if (it == connections_.end()) {

      icontext_t *ctx_ = nullptr;
      if constexpr (std::is_base_of<iobject_t, T>::value)
        ctx_ = o->context();

      return connections_.emplace_back(ctx_, std::move(slot_));
    } else
      return *it;
  }

  template <typename T, typename Dependent_context_t__ = result_type_t__>
  [[nodiscard]] connection_t connect([[maybe_unused]] T *o,
                                     signal_t__ &signal) && = delete;

  void disconnect(signal_t__ &_disconnect_signal) & {

    disconnect<base_t__, &base_t__::operator()>(&_disconnect_signal);
  }
  template <typename T>
  [[nodiscard]] connection_t connect([[maybe_unused]] T *o,
                                     signal_t__ &signal) && = delete;

  void disconnect(const connection_t &_c) & {

    auto it = std::lower_bound(connections_.begin(), connections_.end(), _c,
                               std::less<>{});

    if (it != connections_.end())
      connections_.erase(it);
  }
  void disconnect(const connection_t &_c) && = delete;

  void disconnect() & { connections_.clear(); }
  void disconnect() && = delete;

private:
  template <typename... A> void notify(A &&...p) noexcept {

    auto begin_ = std::begin(connections_);
    auto end_ = std::end(connections_);

    for (; begin_ != end_; ++begin_) {

      auto &connection_ = *begin_;

      if (connection_.result_ && !connection_.result_->is_connected) {
        connections_.erase(begin_);
        continue;
      }

      if (auto ctx_ = connection_.ctx_) {

        typename private_connection_t__::result_ptr_t::weak_type result_weak_ =
            connection_.result_;

        if constexpr (sizeof...(p) > 0) {

          icontext_t::task_t task_(
              [args_ = std::make_tuple(arg_t__<A>{std::forward<A>(p)}...),
               slot_ = connection_.slot_,
               result_weak_ = result_weak_]() mutable {
                if (auto result_ = result_weak_.lock(); result_ && result_->is_connected) {
                  if constexpr (std::is_same_v<result_type_t__, void>)
                    slot_(std::get<arg_t__<A>>(args_)...);
                  else {
                    result_->set(slot_(std::get<arg_t__<A>>(args_)...).value());
                  }
                }
              });

          if (task_) {
            auto res_ = ctx_->schedule(std::move(task_));

            if (res_.error() == detail::queue_status_t::Q_OVERFLOW) {
              connection_.result_->status_ =
                  std::error_code{signal_status_t::S_SLOT_QUEUE_OVERFLOW};
            }
          }
        } else {
          icontext_t::task_t task_(
              [slot_ = connection_.slot_, result_weak_ = result_weak_]() {
                if (auto result_ = result_weak_.lock(); result_ && result_->is_connected) {
                  if constexpr (std::is_same_v<result_type_t__, void>)
                    slot_();
                  else {
                    result_->set(slot_().value());
                  }
                }
              });

          if (task_) {
            auto res_ = ctx_->schedule(std::move(task_));

            if (res_.error() == detail::queue_status_t::Q_OVERFLOW) {
              connection_.result_->status_ =
                  std::error_code{signal_status_t::S_SLOT_QUEUE_OVERFLOW};
            }
          }
        }
      } else {
        if constexpr (std::is_same_v<result_type_t__, void>)
          connection_.slot_(std::forward<A>(p)...);
        else
          connection_.result_->set(
              connection_.slot_(std::forward<A>(p)...).value());
      }
    }
  }

  container_connections_t__ connections_;
};
} // namespace detail

template <typename SIGNATURE> using signal_t = detail::signal_t__<SIGNATURE>;

} // namespace signaler

#endif // SIGNALER_SIGNAL_HPP