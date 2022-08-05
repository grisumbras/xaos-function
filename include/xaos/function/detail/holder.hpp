#ifndef XAOS_FUNCTION_DETAIL_HOLDER_HPP
#define XAOS_FUNCTION_DETAIL_HOLDER_HPP


#include <xaos/function/detail/mpl.hpp>

#include <boost/static_assert.hpp>

#include <memory>


namespace xaos {
namespace function_detail {


template <class Signature, bool IsCloneable>
struct holder_impl_base;

template <class R, class... Args>
struct holder_impl_base<R(Args...), false> {
  virtual ~holder_impl_base() = default;
  virtual auto target() noexcept -> void* = 0;
  virtual auto invoke(Args...) -> R = 0;
};

template <class Signature>
struct holder_impl_base<Signature, true> : holder_impl_base<Signature, false> {
  virtual auto clone() const -> std::unique_ptr<holder_impl_base> = 0;
};


template <class Signature, class F, bool IsCloneable, bool IsBaseCloneable>
struct holder_impl;

template <class R, class... Args, class F, bool IsBaseCloneable>
struct holder_impl<R(Args...), F, false, IsBaseCloneable>
  : holder_impl_base<R(Args...), IsBaseCloneable> {
  BOOST_STATIC_ASSERT(!std::is_lvalue_reference<F>::value);

  using impl_base = holder_impl_base<R(Args...), IsBaseCloneable>;

  F f_;

  explicit holder_impl(F f) : f_(std::move(f)) {}

  auto target() noexcept -> void* override { return std::addressof(f_); }

  auto invoke(Args... args) -> R override
  {
    return f_(static_cast<Args>(args)...);
  }
};


template <class Signature, class F>
struct holder_impl<Signature, F, true, true>
  : holder_impl<Signature, F, false, true> {
  using base_type = holder_impl<Signature, F, false, true>;
  using base_type::base_type;

  auto clone() const -> std::unique_ptr<typename base_type::impl_base> override
  {
    return std::make_unique<holder_impl>(this->f_);
  }
};


template <class Signature, bool IsCopyable>
struct holder;

template <class Signature>
struct holder<Signature, false> {
  using impl_base = holder_impl_base<Signature, false>;

  template <class F>
  holder(F f)
    : holder_(
      std::make_unique<holder_impl<Signature, F, false, false>>(std::move(f)))
  {}

  std::unique_ptr<impl_base> holder_;
};

template <class Signature>
struct holder<Signature, true> {
  using impl_base = holder_impl_base<Signature, true>;

  template <class F>
  holder(F f)
    : holder_(
      std::make_unique<holder_impl<Signature, F, true, true>>(std::move(f)))
  {}

  holder(holder&& other) = default;
  auto operator=(holder&& other) noexcept -> holder& = default;

  holder(holder const& other) : holder_(other.holder_->clone()) {}

  auto operator=(holder const& other) -> holder&
  {
    auto temp = other;
    return (*this) = std::move(temp);
  }

  std::unique_ptr<impl_base> holder_;
};


template <class T>
struct remove_func_const_impl : mpl::mp_identity<T> {};

template <class R, class... Args>
struct remove_func_const_impl<R(Args...) const>
  : remove_func_const_impl<R(Args...)> {};

template <class Signature>
using remove_func_const = typename remove_func_const_impl<Signature>::type;


template <class Signature, bool IsCopyable>
using holder_for = holder<remove_func_const<Signature>, IsCopyable>;


} // namespace function_detail
} // namespace xaos


#endif // XAOS_FUNCTION_DETAIL_HOLDER_HPP
