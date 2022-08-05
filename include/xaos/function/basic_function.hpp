#ifndef XAOS_FUNCTION_BASIC_FUNCTION_HPP
#define XAOS_FUNCTION_BASIC_FUNCTION_HPP


#include <xaos/function/detail/holder.hpp>
#include <xaos/function/detail/invoker.hpp>


namespace xaos {


template <class Signature, bool IsCopyable>
class basic_function
  : function_detail::invoker<basic_function<Signature, IsCopyable>, Signature>
  , function_detail::holder_for<Signature, IsCopyable>
{
private:
  using invoker_base = function_detail::
    invoker<basic_function<Signature, IsCopyable>, Signature>;
  using holder_base = function_detail::holder_for<Signature, IsCopyable>;

  friend invoker_base;

public:
  using signature = Signature;
  using result_type = typename invoker_base::result_type;

  template <class F>
  basic_function(F f);

  template <class F>
  auto target() noexcept -> F*;

  template <class F>
  auto target() const noexcept -> F const*;

  using invoker_base::operator();
};


template <class S, bool C>
template <class F>
basic_function<S, C>::basic_function(F f) : holder_base(std::move(f))
{}


template <class S, bool C>
template <class F>
auto basic_function<S, C>::target() noexcept -> F*
{
  return reinterpret_cast<F*>(this->holder_->target());
}


template <class S, bool C>
template <class F>
auto basic_function<S, C>::target() const noexcept -> F const*
{
  return reinterpret_cast<F const*>(this->holder_->target());
}


} // namespace xaos


#endif // XAOS_FUNCTION_BASIC_FUNCTION_HPP
