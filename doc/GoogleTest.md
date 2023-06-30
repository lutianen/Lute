# GoogleTest

**gtest** 是一个跨平台的(Liunx、Mac OS X、Windows 、Cygwin 、Windows CE and Symbian ) C++单元测试框架，由google公司发布。gtest是为在不同平台上为编写C++测试而生成的。它提供了丰富的断言、致命和非致命判断、参数化、”死亡测试”等等。

## gtest系列之TEST宏

```c++
TEST(test_case_name, test_name)
TEST_F(test_fixture,test_name) //多个测试场景需要相同数据配置的情况，用TEST_F。TEST_F test fixture，测试夹具，测试套，承担了一个注册的功能。
```

TEST宏的作用是创建一个简单测试，它定义了一个测试函数，在这个函数里可以使用任何C++代码并使用提供的断言来进行检查。

实现：

```c++
// 顶层宏 TEST
#define TEST(test_suite_name, test_name) GTEST_TEST(test_suite_name, test_name)

// GTEST_TEST 引入 父类和ID ::testing::Test、::testing::internal::GetTestTypeId()
#define GTEST_TEST(test_suite_name, test_name)             \
  GTEST_TEST_(test_suite_name, test_name, ::testing::Test, \
              ::testing::internal::GetTestTypeId())

// GTEST_TEST_ 宏实现：定义一个继承于 ::testing::Test 的子类，其中 TestBody() 函数
// 实现了自己书写的相关测试代码
// test_info_ 是与该类所有信息相关的变量，被加入到一个链表（vector）中，用来实现TestBody()方法的调用
#define GTEST_TEST_(test_suite_name, test_name, parent_class, parent_id)       \
  static_assert(sizeof(GTEST_STRINGIFY_(test_suite_name)) > 1,                 \
                "test_suite_name must not be empty");                          \
  static_assert(sizeof(GTEST_STRINGIFY_(test_name)) > 1,                       \
                "test_name must not be empty");                                \
  class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                     \
      : public parent_class {                                                  \
   public:                                                                     \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() = default;            \
    ~GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() override = default;  \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                         \
    (const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &) = delete;     \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) & operator=(            \
        const GTEST_TEST_CLASS_NAME_(test_suite_name,                          \
                                     test_name) &) = delete; /* NOLINT */      \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                         \
    (GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &&) noexcept = delete; \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) & operator=(            \
        GTEST_TEST_CLASS_NAME_(test_suite_name,                                \
                               test_name) &&) noexcept = delete; /* NOLINT */  \
                                                                               \
   private:                                                                    \
    void TestBody() override;                                                  \
    static ::testing::TestInfo* const test_info_ GTEST_ATTRIBUTE_UNUSED_;      \
  };                                                                           \
                                                                               \
  ::testing::TestInfo* const GTEST_TEST_CLASS_NAME_(test_suite_name,           \
                                                    test_name)::test_info_ =   \
      ::testing::internal::MakeAndRegisterTestInfo(                            \
          #test_suite_name, #test_name, nullptr, nullptr,                      \
          ::testing::internal::CodeLocation(__FILE__, __LINE__), (parent_id),  \
          ::testing::internal::SuiteApiResolver<                               \
              parent_class>::GetSetUpCaseOrSuite(__FILE__, __LINE__),          \
          ::testing::internal::SuiteApiResolver<                               \
              parent_class>::GetTearDownCaseOrSuite(__FILE__, __LINE__),       \
          new ::testing::internal::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(     \
              test_suite_name, test_name)>);                                   \
  void GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::TestBody()
                                                   
// Creates a new TestInfo object and registers it with Google Test
TestInfo* MakeAndRegisterTestInfo(
    const char* test_suite_name, const char* name, const char* type_param,
    const char* value_param, CodeLocation code_location,
    TypeId fixture_class_id, SetUpTestSuiteFunc set_up_tc,
    TearDownTestSuiteFunc tear_down_tc, TestFactoryBase* factory) {
  TestInfo* const test_info =
      new TestInfo(test_suite_name, name, type_param, value_param,
                   code_location, fixture_class_id, factory);
  GetUnitTestImpl()->AddTestInfo(set_up_tc, tear_down_tc, test_info);
  return test_info;
}
                                                   
// 
void AddTestInfo(internal::SetUpTestSuiteFunc set_up_tc,
                   internal::TearDownTestSuiteFunc tear_down_tc,
                   TestInfo* test_info) {
#if GTEST_HAS_DEATH_TEST
    // In order to support thread-safe death tests, we need to
    // remember the original working directory when the test program
    // was first invoked.  We cannot do this in RUN_ALL_TESTS(), as
    // the user may have changed the current directory before calling
    // RUN_ALL_TESTS().  Therefore we capture the current directory in
    // AddTestInfo(), which is called to register a TEST or TEST_F
    // before main() is reached.
    if (original_working_dir_.IsEmpty()) {
      original_working_dir_.Set(FilePath::GetCurrentDir());
      GTEST_CHECK_(!original_working_dir_.IsEmpty())
          << "Failed to get the current working directory.";
    }
#endif  // GTEST_HAS_DEATH_TEST

    GetTestSuite(test_info->test_suite_name(), test_info->type_param(),
                 set_up_tc, tear_down_tc)
        ->AddTestInfo(test_info);
  }

// Adds a test to this test suite.  Will delete the test upon
// destruction of the TestSuite object.
void TestSuite::AddTestInfo(TestInfo* test_info) {
  test_info_list_.push_back(test_info);
  test_indices_.push_back(static_cast<int>(test_indices_.size()));
}
                                                   
// 最终维护的链表
// The vector of TestInfos in their original order.  It owns the
// elements in the vector.
std::vector<TestInfo*> test_info_list_;
// Provides a level of indirection for the test list to allow easy
// shuffling and restoring the test order.  The i-th element in this
// vector is the index of the i-th test in the shuffled test list.
std::vector<int> test_indices_;
```

---

## **gtest系列之断言**

gtest中断言的宏可以分为两类：一类是ASSERT宏，另一类就是EXPECT宏了。

- ASSERT_系列：如果当前点检测失败则退出当前函数

- EXPECT_系列：如果当前点检测失败则继续往下执行

如果你对自动输出的错误信息不满意的话，也是可以通过operator<<能够在失败的时候打印日志，将一些自定义的信息输出。

> ASSERT_系列：

bool值检查

-  ASSERT_TRUE(参数)，期待结果是true

-  ASSERT_FALSE(参数)，期待结果是false

数值型数据检查

- ASSERT_EQ(参数1，参数2)，传入的是需要比较的两个数 equal

- ASSERT_NE(参数1，参数2)，not equal，不等于才返回true

- ASSERT_LT(参数1，参数2)，less than，小于才返回true

- ASSERT_GT(参数1，参数2)，greater than，大于才返回true

- ASSERT_LE(参数1，参数2)，less equal，小于等于才返回true

- ASSERT_GE(参数1，参数2)，greater equal，大于等于才返回true

字符串检查

- ASSERT_STREQ(expected_str, actual_str)，两个C风格的字符串相等才正确返回

- ASSERT_STRNE(str1, str2)，两个C风格的字符串不相等时才正确返回

- ASSERT_STRCASEEQ(expected_str, actual_str)

- ASSERT_STRCASENE(str1, str2)

> EXPECT_系列，也是具有类似的宏结构

...

---

## **gtest系列之事件机制**

“事件” 本质是框架给你提供了一个机会, 让你能在这样的几个机会来执行你自己定制的代码, 来给测试用例准备/清理数据。gtest提供了多种事件机制，总结一下gtest的事件一共有三种：

1、TestSuite事件

需要写一个类，继承testing::Test，然后实现两个静态方法：SetUpTestCase 方法在第一个TestCase之前执行；TearDownTestCase方法在最后一个TestCase之后执行。

2、TestCase事件

是挂在每个案例执行前后的，需要实现的是SetUp方法和TearDown方法。SetUp方法在每个TestCase之前执行；TearDown方法在每个TestCase之后执行。

3、全局事件

要实现全局事件，必须写一个类，继承testing::Environment类，实现里面的SetUp和TearDown方法。SetUp方法在所有案例执行前执行；TearDown方法在所有案例执行后执行。

> TestSuite事件

。。。

> TestCase事件

```c++
// The fixture for testing class Foo.
class LogTest : public ::testing::Test {
protected:

LogTest() { ... }

virtual ~LogTest() { }

// If the constructor and destructor are not enough for setting up
// and cleaning up each test, you can define the following methods:

virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
}

virtual void TearDown() {
// Code here will be called immediately after each test (right
// before the destructor).
}

// Objects declared here can be used by all tests in the test case for Foo.
...
};
```

> 全局事件, 除了要继承testing::Environment类，还要定义一个该全局环境的一个对象并将该对象添加到全局环境测试中去。

```c++
class GlobalTest : public testing::Environment {
public:
    virtual void SetUp() { ... }
    virtual void TearDown() { ... }
};

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::Environment* env = new GlobalTest();
    testing::AddGlobalTestEnvironment(env);
    
    return RUN_ALL_TESTS();
}
```

---

## **gtest系列之死亡测试**

这里的”死亡”指的是程序的奔溃。通常在测试的过程中，我们需要考虑各种各样的输入，有的输入可能直接导致程序奔溃，这个时候我们就要检查程序是否按照预期的方式挂掉，这也就是所谓的”死亡测试”。

死亡测试所用到的宏：

1、ASSERT_DEATH(参数1，参数2)，程序挂了并且错误信息和参数2匹配，此时认为测试通过。如果参数2为空字符串，则只需要看程序挂没挂即可。

2、ASSERT_EXIT(参数1，参数2，参数3)，语句停止并且错误信息和被提前给的信息匹配。

## 限制

Google测试旨在线程安全。 在pthreads库可用的系统上，实现是线程安全的。 目前，在其他系统（例如Windows）上同时使用两个线程的Google Test断言是不安全的。 在大多数测试中，这不是一个问题，因为通常断言是在主线程中完成的。 如果你想帮助，你可以志愿为您的平台在gtest-port.h中实现必要的同步原语。
