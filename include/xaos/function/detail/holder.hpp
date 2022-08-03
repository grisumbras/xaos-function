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
using holder_for = holder_base<remove_func_const<Signature>>;


template <class Signature, class F>
struct holder;

template <class R, class... Args, class F>
struct holder<R(Args...), F> : holder_base<R(Args...)> {
  BOOST_STATIC_ASSERT(!std::is_lvalue_reference<F>::value);

  F f;

  holder(F f);
  ~holder() override = default;

  auto target() noexcept -> void* override;
  auto invoke(Args... args) -> R override;
};


template <class R, class... Args, class F>
holder<R(Args...), F>::holder(F f) : f(std::move(f))
{}


template <class R, class... Args, class F>
auto holder<R(Args...), F>::target() noexcept -> void*
{
  return std::addressof(f);
}


template <class R, class... Args, class F>
auto holder<R(Args...), F>::invoke(Args... args) -> R
{
  return f(static_cast<Args>(args)...);
}


template <class Signature, class F>
auto make_holder(F& f) -> std::unique_ptr<holder_for<Signature>>
{
  return std::make_unique<holder<remove_func_const<Signature>, F>>(
    std::move(f));
}


} // namespace function_detail
} // namespace xaos


#endif // XAOS_FUNCTION_DETAIL_HOLDER_HPP
