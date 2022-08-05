#ifndef XAOS_FUNCTION_FUNCTION_ERROR_HPP
#define XAOS_FUNCTION_FUNCTION_ERROR_HPP


#include <boost/system/error_code.hpp>


namespace xaos {


enum class function_error {
  empty_function_called = 1,
};


inline namespace {


class function_error_category_t : public boost::system::error_category
{
public:
  constexpr function_error_category_t() noexcept;

  inline auto name() const noexcept -> char const*;
  inline auto message(int ev) const -> std::string;
  inline auto message(int ev, char* buffer, std::size_t len) const noexcept
    -> char const*;
};


auto function_error_category() noexcept -> boost::system::error_category const&
{
  static function_error_category_t result;
  return result;
}


constexpr function_error_category_t::function_error_category_t() noexcept
  : boost::system::error_category(0xFB6B5786E902200D)
{}


auto function_error_category_t::name() const noexcept -> char const*
{
  return "xaos.function";
}


auto function_error_category_t::message(int ev) const -> std::string
{
  return this->message(ev, nullptr, 0);
}


auto function_error_category_t::message(
  int ev, char*, std::size_t) const noexcept -> char const*
{
  auto const e = static_cast<function_error>(ev);
  switch (e) {
  case function_error::empty_function_called: return "empty function called";
  default: return "unknown error";
  }
}


} // namespace


inline auto make_error_code(function_error e) noexcept
  -> boost::system::error_code
{
  return {static_cast<int>(e), function_error_category()};
}


} // namespace xaos


namespace boost {
namespace system {

template <>
struct is_error_code_enum<xaos::function_error> : std::true_type {};

} // namespace system
} // namespace boost


#endif // XAOS_FUNCTION_FUNCTION_ERROR_HPP
