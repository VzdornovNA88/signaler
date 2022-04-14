#ifndef SIGNALER_STORAGE_POLY_ALLOC_NOEXCEPT_HPP
#define SIGNALER_STORAGE_POLY_ALLOC_NOEXCEPT_HPP

#include <iostream>
#include <utility>
#include "memory_resource_noexcept.hpp"

namespace signaler::detail {

class new_delete_noexcept_resource_t__ : public memmory_resource_noexcept_t {
protected:
  void *do_allocate(size_t bytes, size_t alignment) noexcept override {
    return operator new (bytes, static_cast<std::align_val_t>(alignment),
                         std::nothrow_t{});
  }
  void do_deallocate(void *p, [[maybe_unused]] size_t bytes,
                     size_t alignment) noexcept override {
    operator delete (p, static_cast<std::align_val_t>(alignment),
                     std::nothrow_t{});
  }
  bool do_is_equal(
      const memmory_resource_noexcept_t &other) const noexcept override {
    return this == &other;
  };
};

[[nodiscard]] inline memmory_resource_noexcept_t &default_resource() noexcept {
  static new_delete_noexcept_resource_t__ new_delete_noexcept_resource_;
  return new_delete_noexcept_resource_;
}

template<typename T>
class polymorphic_allocator_noexcept_t {
public:
template <class>
        friend class polymorphic_allocator_noexcept_t;

  using value_type = T;

  polymorphic_allocator_noexcept_t() noexcept = default;
  polymorphic_allocator_noexcept_t(
      memmory_resource_noexcept_t *const res) noexcept
      : resource_{res == nullptr ? &default_resource() : res} {}
  polymorphic_allocator_noexcept_t(
      const polymorphic_allocator_noexcept_t &) noexcept = default;
  polymorphic_allocator_noexcept_t &
  operator=(const polymorphic_allocator_noexcept_t &) noexcept = delete;

  [[nodiscard]] T *allocate(const size_t count) noexcept {
    void *const p_ = resource_->allocate(sizeof(T) * count, alignof(T));
    return static_cast<T *>(p_);
  }

  void deallocate(T *const p, const size_t count) noexcept {
    resource_->deallocate(p, count * sizeof(T), alignof(T));
  }

  template <class U, class... Args>
  void construct(U* p, Args&&... args) const noexcept(noexcept(U(std::forward<Args>(args)...))) {
    new (p) U(std::forward<Args>(args)...);
  }

  template<class U>
  void destroy(U* p) const noexcept {
    p->~U();
  }

  [[nodiscard]] polymorphic_allocator_noexcept_t select_on_container_copy_construction() const noexcept {
            return {};
  }

  [[nodiscard]] memmory_resource_noexcept_t *resource() const noexcept {
    return resource_;
  }
private:
  memmory_resource_noexcept_t *resource_ = &default_resource();
};

template <class T1, class T2>
    [[nodiscard]] bool operator==(
        const polymorphic_allocator_noexcept_t<T1>& l, const polymorphic_allocator_noexcept_t<T2>& r) noexcept {
        return *l.resource() == *r.resource();
    }

} // namespace signaler::detail

#endif // SIGNALER_STORAGE_POLY_ALLOC_NOEXCEPT_HPP