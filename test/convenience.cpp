#include <xaos/function.hpp>

#include <boost/core/lightweight_test.hpp>


int main()
{
  (void)xaos::function<void()>([] {});
  return boost::report_errors();
}
