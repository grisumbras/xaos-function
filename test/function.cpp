#include <xaos/function/copyable_function.hpp>
#include <xaos/function/function.hpp>

#include <boost/core/lightweight_test.hpp>
#include <boost/core/lightweight_test_trait.hpp>

#include <iostream>


template <template <class...> class Function>
void test_common()
{
  // void()
  {
    auto called = false;
    auto f = Function<void()>([&called] { called = true; });
    BOOST_TEST(f);
    BOOST_TEST_NOT(called);
    f();
    BOOST_TEST(called);
  }
  // void(arg)
  {
    int output = 0;
    auto f = Function<void(int)>([&output](int n) { output = n; });
    BOOST_TEST(f);
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
    auto f = Function<int(int)>([](int n) { return 2 * n; });
    BOOST_TEST(f);
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
    auto f = Function<int()>([n = 0]() mutable { return ++n * 2; });
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
      auto f = Function<int()>(
        [state = std::move(state)]() mutable { return ++(*state); });
      BOOST_TEST_NOT(called);
      BOOST_TEST_EQ(f(), 1);
      BOOST_TEST_NOT(called);
    }

    BOOST_TEST(called);
  }
  // const-qualified operator()
  {
    auto const f = Function<int(int) const>([](int n) { return n; });
    BOOST_TEST_EQ(f(1), 1);
    BOOST_TEST_EQ(f(2), 2);
    BOOST_TEST_EQ(f(3), 3);
  }
  // target
  {
    int n = 0;
    auto func = [&n]() { ++n; };
    auto f = xaos::function<void()>(func);
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
  // default constructor
  {
    auto f = Function<int(int)>();
    BOOST_TEST_NOT(f);
    try {
      f.invoke(1);
    } catch (boost::system::system_error const& e) {
      BOOST_TEST(e.code() == xaos::function_error::empty_function_called);
    }

    f = [](int n) { return n * n; };
    BOOST_TEST(f);
    BOOST_TEST_EQ(f(5), 25);
    BOOST_TEST_EQ(f.invoke(6), 36);
  }
  // move
  {
    auto f = Function<int(int)>([](int n) { return n; });

    auto g = std::move(f);
    BOOST_TEST_NOT(f);
    BOOST_TEST(g);
  }
}


int main()
{
  namespace d = xaos::function_detail;
  BOOST_TEST_TRAIT_SAME(
    d::vtable_for<void(), false>,
    std::tuple<
      void (*)(void*),
      void* (*)(void*),
      void (*)(void*),
      void (*)(void*, void*)>);
  BOOST_TEST_TRAIT_SAME(
    d::vtable_for<void(), true>,
    std::tuple<
      void (*)(void*),
      void* (*)(void*),
      void (*)(void*),
      void (*)(void*, void*),
      void (*)(void*, void*)>);
  BOOST_TEST_TRAIT_SAME(
    d::vtable_for<int(double, float const&), true>,
    std::tuple<
      int (*)(void*, double, float const&),
      void* (*)(void*),
      void (*)(void*),
      void (*)(void*, void*),
      void (*)(void*, void*)>);

  test_common<xaos::function>();
  // non-copyable f
  {
    auto state = std::make_unique<int>(0);
    auto f = xaos::function<int()>(
      [state = std::move(state)]() mutable { return ++(*state); });
    BOOST_TEST_EQ(f(), 1);
    BOOST_TEST_EQ(f(), 2);
    BOOST_TEST_EQ(f(), 3);
  }

  test_common<xaos::copyable_function>();
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
