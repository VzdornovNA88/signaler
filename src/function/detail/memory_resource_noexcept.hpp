#ifndef SIGNALER_STORAGE_MEM_RESOURCE_NOEXCEPT_HPP
#define SIGNALER_STORAGE_MEM_RESOURCE_NOEXCEPT_HPP

#include <cstddef>
#include <memory>

namespace signaler {

class memmory_resource_noexcept_t {
public:
  virtual ~memmory_resource_noexcept_t() noexcept = default;

  [[nodiscard]] void *
  allocate(const size_t bytes,
           const size_t al = alignof(max_align_t)) noexcept {
    return do_allocate(bytes, al);
  }

  void deallocate(void *const p, const size_t bytes,
                  const size_t al = alignof(max_align_t)) noexcept {
    return do_deallocate(p, bytes, al);
  }

  [[nodiscard]] bool
  is_equal(const memmory_resource_noexcept_t &that) const noexcept {
    return do_is_equal(that);
  }

private:
  virtual void *do_allocate(size_t bytes, size_t al) noexcept = 0;
  virtual void do_deallocate(void *p, size_t bytes, size_t al) noexcept = 0;
  virtual bool
  do_is_equal(const memmory_resource_noexcept_t &that) const noexcept = 0;
};

[[nodiscard]] inline bool
operator==(const memmory_resource_noexcept_t &l,
           const memmory_resource_noexcept_t &r) noexcept {
  return &l == &r || l.is_equal(r);
}

[[nodiscard]] inline bool
operator!=(const memmory_resource_noexcept_t &l,
           const memmory_resource_noexcept_t &r) noexcept {
  return !(l == r);
}
} // namespace signaler::detail

#endif // SIGNALER_STORAGE_MEM_RESOURCE_NOEXCEPT_HPP