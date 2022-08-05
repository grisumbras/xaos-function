#ifndef XAOS_FUNCTION_FUNCTION_HPP
#define XAOS_FUNCTION_FUNCTION_HPP


#include <xaos/function/basic_function.hpp>


namespace xaos {


template <class Signature>
using function = basic_function<Signature, false>;


} // namespace xaos


#endif // XAOS_FUNCTION_FUNCTION_HPP
