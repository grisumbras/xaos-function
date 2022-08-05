#ifndef XAOS_FUNCTION_DETAIL_HOLDER_HPP
#define XAOS_FUNCTION_DETAIL_HOLDER_HPP


#include <xaos/function/detail/mpl.hpp>

#include <boost/static_assert.hpp>

#include <memory>


namespace xaos {
namespace function_detail {

template <class T>
struct remove_func_const_impl : mpl::mp_identity<T> {};

template <class R, class... Args>
struct remove_func_const_impl<R(Args...) const>
  : remove_func_const_impl<R(Args...)> {};

template <class Signature>
using remove_func_const = typename remove_func_const_impl<Signature>::type;


template <class Signature>
struct holder_base;

template <class R, class... Args>
struct holder_base<R(Args...)> {
  virtual ~holder_base() = default;
  virtual auto target() noexcept -> void* = 0;
  virtual auto invoke(Args...) -> R = 0;
};


template <class Signature>
struct cloneable_holder_base : holder_base<Signature> {
  virtual auto clone() const -> std::unique_ptr<cloneable_holder_base> = 0;
};


template <
  bool IsCopyable,
  template <class...>
  class L1,
  template <class...>
  class L2,
  class Signature,
  class... Args>
using holder_for_impl = mpl::mp_invoke_q<
  mpl::mp_if_c<IsCopyable, mpl::mp_quote<L1>, mpl::mp_quote<L2>>,
  remove_func_const<Signature>,
  Args...>;

template <bool IsCopyable, class Signature>
using holder_base_for
  = holder_for_impl<IsCopyable, cloneable_holder_base, holder_base, Signature>;


template <bool IsCopyable, bool IsDerivedCopyable, class Signature, class F>
struct holder_impl;

template <bool IsDerivedCopyable, class R, class... Args, class F>
struct holder_impl<false, IsDerivedCopyable, R(Args...), F>
  : holder_base_for<IsDerivedCopyable, R(Args...)> {
  BOOST_STATIC_ASSERT(!std::is_lvalue_reference<F>::value);

  F f;

  explicit holder_impl(F f);

  auto target() noexcept -> void* override;
  auto invoke(Args... args) -> R override;
};


template <class R, class... Args, class F>
struct holder_impl<true, true, R(Args...), F>
  : holder_impl<false, true, R(Args...), F> {
  using base_type = holder_impl<false, true, R(Args...), F>;
  using base_type::base_type;

  auto clone() const
    -> std::unique_ptr<typename holder_impl::cloneable_holder_base> override;
};


template <class Signature, class F>
using holder = holder_impl<false, false, Signature, F>;

template <class Signature, class F>
using cloneable_holder = holder_impl<true, true, Signature, F>;

template <bool IsCopyable, class Signature, class F>
using holder_for
  = holder_for_impl<IsCopyable, cloneable_holder, holder, Signature, F>;


template <bool C, class R, class... Args, class F>
holder_impl<false, C, R(Args...), F>::holder_impl(F f) : f(std::move(f))
{}


template <bool C, class R, class... Args, class F>
auto holder_impl<false, C, R(Args...), F>::target() noexcept -> void*
{
  return std::addressof(f);
}


template <bool C, class R, class... Args, class F>
auto holder_impl<false, C, R(Args...), F>::invoke(Args... args) -> R
{
  return f(static_cast<Args>(args)...);
}


template <class R, class... Args, class F>
auto holder_impl<true, true, R(Args...), F>::clone() const
  -> std::unique_ptr<typename holder_impl::cloneable_holder_base>
{
  return std::make_unique<holder_impl>(this->f);
}


template <bool IsCopyable, class Signature, class F>
auto make_holder(F& f)
  -> std::unique_ptr<holder_base_for<IsCopyable, Signature>>
{
  using Holder = holder_for<IsCopyable, Signature, F>;
  return std::make_unique<Holder>(std::move(f));
}


} // namespace function_detail
} // namespace xaos


#endif // XAOS_FUNCTION_DETAIL_HOLDER_HPP
