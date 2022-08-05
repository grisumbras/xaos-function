#ifndef XAOS_FUNCTION_COPYABLE_FUNCTION_HPP
#define XAOS_FUNCTION_COPYABLE_FUNCTION_HPP


#include <xaos/function/detail/holder.hpp>
#include <xaos/function/detail/invoker.hpp>

#include <memory>


namespace xaos {


template <class Signature>
class copyable_function
  : function_detail::invoker<copyable_function<Signature>, Signature>
{
private:
  using invoker_base
    = function_detail::invoker<copyable_function<Signature>, Signature>;

public:
  using signature = Signature;
  using result_type = typename invoker_base::result_type;

  using invoker_base::operator();

  template <class F>
  copyable_function(F f);

  copyable_function(copyable_function&& other) = default;
  auto operator=(copyable_function&& other) noexcept
    -> copyable_function& = default;

  copyable_function(copyable_function const& other);
  auto operator=(copyable_function const& other) -> copyable_function&;

  template <class F>
  auto target() noexcept -> F*;

  template <class F>
  auto target() const noexcept -> F const*;

private:
  friend invoker_base;

  using holder_base = function_detail::holder_base_for<true, Signature>;

  std::unique_ptr<holder_base> holder_;
};


template <class S>
template <class F>
copyable_function<S>::copyable_function(F f)
  : holder_(function_detail::make_holder<true, S>(f))
{}


template <class S>
copyable_function<S>::copyable_function(copyable_function const& other)
  : holder_(other.holder_->clone())
{}


template <class S>
auto copyable_function<S>::operator=(copyable_function const& other)
  -> copyable_function&
{
  auto temp = other;
  return (*this) = std::move(temp);
}


template <class S>
template <class F>
auto copyable_function<S>::target() noexcept -> F*
{
  return reinterpret_cast<F*>(holder_->target());
}


template <class S>
template <class F>
auto copyable_function<S>::target() const noexcept -> F const*
{
  return reinterpret_cast<F const*>(holder_->target());
}


} // namespace xaos


#endif // XAOS_FUNCTION_COPYABLE_FUNCTION_HPP
