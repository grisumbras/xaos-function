#ifndef XAOS_FUNCTION_FUNCTION_HPP
#define XAOS_FUNCTION_FUNCTION_HPP


#include <xaos/function/detail/holder.hpp>
#include <xaos/function/detail/invoker.hpp>

#include <memory>


namespace xaos {


template <class Signature>
class function;


template <class Signature>
class function : function_detail::invoker<function<Signature>, Signature>
{
private:
  using invoker_base
    = function_detail::invoker<function<Signature>, Signature>;

public:
  using signature = Signature;
  using result_type = typename invoker_base::result_type;

  using invoker_base::operator();

  template <class F>
  function(F f);

  template <class F>
  auto target() noexcept -> F*;

  template <class F>
  auto target() const noexcept -> F const*;

private:
  friend invoker_base;

  using holder_base = function_detail::holder_base_for<false, Signature>;

  std::unique_ptr<holder_base> holder_;
};


template <class Signature>
template <class F>
function<Signature>::function(F f)
  : holder_(function_detail::make_holder<false, Signature>(f))
{}


template <class Signature>
template <class F>
auto function<Signature>::target() noexcept -> F*
{
  return reinterpret_cast<F*>(holder_->target());
}


template <class Signature>
template <class F>
auto function<Signature>::target() const noexcept -> F const*
{
  return reinterpret_cast<F const*>(holder_->target());
}


} // namespace xaos


#endif // XAOS_FUNCTION_FUNCTION_HPP
