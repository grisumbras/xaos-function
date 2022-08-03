#ifndef XAOS_FUNCTION_DETAIL_INVOKER_HPP
#define XAOS_FUNCTION_DETAIL_INVOKER_HPP


namespace xaos {
namespace function_detail {


template <class Derived, class Signature>
struct invoker;


template <class Derived, class R, class... Args>
struct invoker<Derived, R(Args...)> {
  using result_type = R;

  auto operator()(Args...) -> result_type;
};


template <class Derived, class R, class... Args>
struct invoker<Derived, R(Args...) const> : invoker<Derived, R(Args...)> {
  using base = invoker<Derived, R(Args...)>;
  using result_type = typename base::result_type;

  using base::operator();
  auto operator()(Args...) const -> result_type;
};


template <class D, class R, class... Args>
auto invoker<D, R(Args...)>::operator()(Args... args) -> result_type
{
  return static_cast<D&>(*this).holder_->invoke(static_cast<Args>(args)...);
}


template <class D, class R, class... Args>
auto invoker<D, R(Args...) const>::operator()(Args... args) const
  -> result_type
{
  return static_cast<D const&>(*this).holder_->invoke(
    static_cast<Args>(args)...);
}


} // namespace function_detail
} // namespace xaos


#endif // XAOS_FUNCTION_DETAIL_INVOKER_HPP
