#ifndef XAOS_FUNCTION_COPYABLE_FUNCTION_HPP
#define XAOS_FUNCTION_COPYABLE_FUNCTION_HPP


#include <xaos/function/basic_function.hpp>


namespace xaos {


template <class Signature>
using copyable_function = basic_function<Signature, true>;


} // namespace xaos


#endif // XAOS_FUNCTION_COPYABLE_FUNCTION_HPP
