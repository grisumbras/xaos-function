#include <xaos/function/copyable_function.hpp>

#include <boost/core/lightweight_test.hpp>


int main()
{
  // void()
  {
    auto called = false;
    auto f = xaos::copyable_function<void()>([&called] { called = true; });
    BOOST_TEST_NOT(called);
    f();
    BOOST_TEST(called);
  }
  // void(arg)
  {
    int output = 0;
    auto f
      = xaos::copyable_function<void(int)>([&output](int n) { output = n; });
    BOOST_TEST_EQ(output, 0);
    f(1);
    BOOST_TEST_EQ(output, 1);
    f(10);
    BOOST_TEST_EQ(output, 10);
    f(1337);
    BOOST_TEST_EQ(output, 1337);
  }
  // R(arg)
  {
    auto f = xaos::copyable_function<int(int)>([](int n) { return 2 * n; });
    BOOST_TEST_EQ(f(1), 2);
    BOOST_TEST_EQ(f(2), 4);
    BOOST_TEST_EQ(f(3), 6);

    f = [](int n) { return n * n; };
    BOOST_TEST_EQ(f(1), 1);
    BOOST_TEST_EQ(f(2), 4);
    BOOST_TEST_EQ(f(3), 9);
  }
  // mutable
  {
    auto f
      = xaos::copyable_function<int()>([n = 0]() mutable { return ++n * 2; });
    BOOST_TEST_EQ(f(), 2);
    BOOST_TEST_EQ(f(), 4);
    BOOST_TEST_EQ(f(), 6);
  }
  // destuctor for f called
  {
    auto called = false;
    auto deleter = [&called](auto ptr) {
      delete ptr;
      called = true;
    };
    auto state = std::shared_ptr<int>(new int(), deleter);
    BOOST_TEST_NOT(called);

    {
      auto f = xaos::copyable_function<int()>(
        [state = std::move(state)]() mutable { return ++(*state); });
      BOOST_TEST_NOT(called);
      BOOST_TEST_EQ(f(), 1);
      BOOST_TEST_NOT(called);
    }

    BOOST_TEST(called);
  }
  // const-qualified operator()
  {
    auto const f
      = xaos::copyable_function<int(int) const>([](int n) { return n; });
    BOOST_TEST_EQ(f(1), 1);
    BOOST_TEST_EQ(f(2), 2);
    BOOST_TEST_EQ(f(3), 3);
  }
  // target
  {
    int n = 0;
    auto func = [&n]() { ++n; };
    auto f = xaos::copyable_function<void()>(func);
    f();
    BOOST_TEST_EQ(n, 1);

    auto& target = *f.target<decltype(func)>();
    target();
    BOOST_TEST_EQ(n, 2);

    auto const& cf = f;
    auto const& ctarget = *cf.target<decltype(func)>();
    ctarget();
    BOOST_TEST_EQ(n, 3);
    BOOST_TEST_EQ(&target, &ctarget);
  }
  // copy
  {
    auto func = [] {};
    auto const f_ptr = +func;
    auto const f = xaos::copyable_function<void()>(f_ptr);
    auto const g = f;
    BOOST_TEST_EQ(*f.target<decltype(f_ptr)>(), *g.target<decltype(f_ptr)>());
  }

  return boost::report_errors();
}
