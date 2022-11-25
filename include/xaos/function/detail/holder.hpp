#ifndef XAOS_FUNCTION_DETAIL_HOLDER_HPP
#define XAOS_FUNCTION_DETAIL_HOLDER_HPP


#include <xaos/function/detail/mpl.hpp>

#include <boost/core/empty_value.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/static_assert.hpp>

#include <memory>


namespace xaos {
namespace function_detail {


struct some_class;


using small_types = mpl::mp_list<
  void (*)(),
  void (*const)(),
  void (some_class::*)(),
  void (some_class::*const)(),
  some_class*,
  some_class* const>;

template <class T, class U>
using compare_size = mpl::mp_bool<sizeof(T) < sizeof(U)>;
using maximum_size = mpl::mp_max_element<small_types, compare_size>;

template <class T, class U>
using compare_alignment = mpl::mp_bool<alignof(T) < alignof(U)>;
using maximum_alignment = mpl::mp_max_element<small_types, compare_alignment>;


template <class Signature>
struct invoke_t_impl;

template <class R, class... Args>
struct invoke_t_impl<R(Args...)> : mpl::mp_identity<R (*)(void*, Args...)> {};

template <class Signature>
using invoke_t = typename invoke_t_impl<Signature>::type;
using get_target_t = void* (*)(void*);
using destroy_t = void (*)(void*);
using move_t = void (*)(void*, void*);
using copy_t = void (*)(void*, void*);


template <bool IsCopyable, class Base>
using operations_for_impl
  = mpl::mp_eval_if_c<!IsCopyable, Base, mpl::mp_push_back, Base, copy_t>;

template <bool IsCopyable, class Signature>
using operations_for = operations_for_impl<
  IsCopyable,
  mpl::mp_list<invoke_t<Signature>, get_target_t, destroy_t, move_t>>;

template <class Signature, bool IsCopyable>
using vtable_for
  = mpl::mp_apply<std::tuple, operations_for<IsCopyable, Signature>>;


template <class Signature, bool IsCopyable, class T>
struct vtable;

template <class R, class... Args, bool IsCopyable, class T>
struct vtable<R(Args...), IsCopyable, T> {
  static auto target(void*) -> void*;

  template <class... As>
  static void create(void*, As&&...);

  static void destroy(void*) noexcept;
  static void move(void*, void*) noexcept;
  static void copy(void*, void*);

  static auto invoke(void*, Args...) -> R;
};


template <class R, class... As, bool C, class T>
auto vtable<R(As...), C, T>::target(void* storage) -> void*
{
  using Ptr = T*;
  auto sz = std::size_t();
  auto const result
    = std::align(sizeof(Ptr), alignof(Ptr), storage, maximum_size::value);
  BOOST_ASSERT(result);
  return result;
}


template <class R, class... As, bool C, class T>
template <class... Bs>
void vtable<R(Args...), C, T>::create(void* storage, Bs&&... args)
{
  auto const ptr = new T(static_cast<Bs&&>(args)...);
  auto const ptr_ptr = reinterpret_cast<T**>(target(storage));
  new (ptr_ptr) T*(ptr);
}


template <class R, class... Args, bool C, class T>
void vtable<R(Args...), C, T>::destroy(void* storage)
{
  auto const ptr = reinterpret_cast<T**>(target(storage));
  delete ptr;
}


template <class R, class... Args, bool C, class T>
void vtable<R(Args...), C, T>::move(void* raw_l, void* raw_r)
{
  auto& l = *reinterpret_cast<T*>(raw_l);
  auto& r = *reinterpret_cast<T*>(raw_r);
  l = std::move(r);
}


template <class R, class... Args, class T>
void vtable<R(Args...), true, T>::copy(void* raw_l, void* raw_r)
{
  auto& l = *reinterpret_cast<T**>(target(raw_l));
  auto& r = *reinterpret_cast<T**>(target(raw_r));
  *l = *r;
}


// template <class Signature, bool IsCopyable>
// using vtable_getter = auto(*)() -> vtable_for<Signature, IsCopyable>;


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
  : holder_impl_base<R(Args...), IsBaseCloneable>
  , boost::empty_value<F, 0> {
  BOOST_STATIC_ASSERT(!std::is_lvalue_reference<F>::value);

  using impl_base = holder_impl_base<R(Args...), IsBaseCloneable>;
  using func_base = boost::empty_value<F, 0>;

  explicit holder_impl(F f) : func_base(boost::empty_init_t(), std::move(f)) {}

  auto target() noexcept -> void* override
  {
    return std::addressof(func_base::get());
  }

  auto invoke(Args... args) -> R override
  {
    return func_base::get()(static_cast<Args>(args)...);
  }
};


template <class Signature, class F>
struct holder_impl<Signature, F, true, true>
  : holder_impl<Signature, F, false, true> {
  using base_type = holder_impl<Signature, F, false, true>;
  using base_type::base_type;

  auto clone() const -> std::unique_ptr<typename base_type::impl_base> override
  {
    return std::make_unique<holder_impl>(base_type::func_base::get());
  }
};


template <class Signature, bool IsCopyable>
struct holder;

template <class Signature>
struct holder<Signature, false> {
  using impl_base = holder_impl_base<Signature, false>;


  holder() noexcept = default;

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


  holder() noexcept = default;

  template <class F>
  holder(F f)
    : /*get_vtable_(make_vtable_getter<F>())
    ,*/
    holder_(
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

  // alignas(maximum_alignment::value) unsigned char buf_[maximum_size::value];
  // vtable_getter<Signature, true> get_vtable_;
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
