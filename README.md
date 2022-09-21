# LuCC

> The project LuCC for CXX, including a web server called Lux with inspiring from muduo.

## 特点

- “基于事件编程”：程序主体是被动等待事件发生，事件发生之后网络库会调用（回调）事先注册的事件处理函数（event handler）
- 

## nc (netcat)

1. nc 命令，用于设置路由器，短小精干、功能强大，有着“瑞士军刀”的美誉。

2. 能通过 TCP 和 UDP 在网络中读写数据，主要用来快速构建网络连接：

   - 可以让它以服务器的方式运行，监听某个端口并接收客户连接
   - 也可以使之以客户端方式运行，向服务器发起连接并收发数据

3. 语法：

   ```bash
   nc [-hlnruz][-g<网关...>][-G<指向器数目>][-i<延迟秒数>][-o<输出文件>][-p<通信端口>]
   [-s<来源位址>][-v...][-w<超时秒数>][主机名称][通信端口...]
   ```

4. 常用命令包括：

   - `-i`：设置数据包传送的时间间隔

   - `-v`：显示指令执行过程

   - `-l`：以服务器方式运行，监听指定的端口。nc默认以客户端方式运行

   - `-k`：重复接受并处理某个端口上的所有连接，必须和 `-l` 选项配合使用

   - `-n`：使用 IP 地址表示主机，而不是主机名；使用数字表示端口号，而不是服务名称

   - `-p`：当 nc 命令以客户端方式运行时，强制其使用指定的端口号

   - `-s`：设置本地主机发送的数据包的IP地址

   - `-C`：将 `CR` 和 `LF` 两个字符作为行结束符

   - `-U`：使用 UNIX 本地域协议通信

   - `-u`：使用 UDP 协议；nc 默认使用 TCP 协议

   - `-w`：如果 nc 客户端在指定时间内未检测到任何输入，则退出

   - `-X`：当 nv 客户端和代理服务器通信时，该选项指定他们之间使用的通信协议；

     目前支持的代理协议包括："4"(SOCKS v.4)，"5"(SOCKS v.5) 和"connect"(HTTPS proxy)；默认使用SOCKS v.5

   - `-x`：指定目标代理服务器的 IP 地址和端口号

   - `-z`：扫描目标机器上的某个或某些服务是否开启（端口扫描）

   - `-g`：<网关> # 设置路由器跃程通信网关，最多可设置8个。

   - `-h `：在线帮助

   - `-o`：<输出文件> # 指定文件名称，把往来传输的数据以16进制字码倾倒成该文件保存。

   - `-r`： 乱数指定本地与远端主机的通信端口。

5. 实例

   1. **TCP端口扫描**

      ```bash
      # 扫描 10.196.124.133 的端口 范围是 1-100
      nc -v -z -w2 10.196.124.133 1-100
      ```

   2. **文件传输**

      ```bash
      # 接收方 提前设置监听端口与要接收的文件名（文件名可自定义）：
      nc -lp 8888 > node.tar.gz
      
      # 传输方 发文件：
      nc -nv 192.168.75.121 8888  < node_exporter-1.3.1.linux-amd64.tar.gz
      # ⚠️ 注意：192.168.75.121是接收方的ip地址。
      ```

      ```bash
      # 如果希望文件传输结束后自动退出，可以使用下面的命令：
      nc -lp 8888 > node.tar.gz
      nc -nv 192.168.75.121 8888 -i 1 < node_exporter-1.3.1.linux-amd64.tar.gz
      # ⚠️ 注意：-i 表示闲置超时时间
      ```

## googletest

gtest是一个跨平台的(Liunx、Mac OS X、Windows 、Cygwin 、Windows CE and Symbian ) C++单元测试框架，由google公司发布。gtest是为在不同平台上为编写C++测试而生成的。它提供了丰富的断言、致命和非致命判断、参数化、”死亡测试”等等。

### gtest系列之TEST宏

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

### **gtest系列之断言**

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

### **gtest系列之事件机制**

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

### **gtest系列之死亡测试**

这里的”死亡”指的是程序的奔溃。通常在测试的过程中，我们需要考虑各种各样的输入，有的输入可能直接导致程序奔溃，这个时候我们就要检查程序是否按照预期的方式挂掉，这也就是所谓的”死亡测试”。

死亡测试所用到的宏：

1、ASSERT_DEATH(参数1，参数2)，程序挂了并且错误信息和参数2匹配，此时认为测试通过。如果参数2为空字符串，则只需要看程序挂没挂即可。

2、ASSERT_EXIT(参数1，参数2，参数3)，语句停止并且错误信息和被提前给的信息匹配。

### 限制

Google测试旨在线程安全。 在pthreads库可用的系统上，实现是线程安全的。 目前，在其他系统（例如Windows）上同时使用两个线程的Google Test断言是不安全的。 在大多数测试中，这不是一个问题，因为通常断言是在主线程中完成的。 如果你想帮助，你可以志愿为您的平台在gtest-port.h中实现必要的同步原语。

## Timestamp

- 继承自 `copyable`类，即 **默认构造、析构函数** 为编译器默认提供版，可以拷贝

- `Timestamp`类维护的私有成员：

  - 精度：毫秒级

  ```c ++
  /// @brief the number of micro seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC). (微秒)
  int64_t microSecondsSinceEpoch_;
  ```

- 核心函数：

  ```c++
  /**
   * @brief Get the LuCCTimestame since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
   * Must use the `toString` or `toFormatStrint` to format
   * @return Timestamp 
   */
  Timestamp
  Timestamp::now() {
      struct timeval tv;
      ::gettimeofday(&tv, NULL);
      int64_t seconds = tv.tv_sec + DELTA_SECONDS_FROM_LOCAL_TIMEZONE_TO_ZORO;    
      return Timestamp(seconds * 1000 * 1000 + tv.tv_usec);
  }
  ```

- 维护`swap(Timestamp& that) { std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_); }`函数，属于不抛出异常类型函数，对应于《Effective C++》中的条款25：考虑写出一个不抛异常的 swap 函数。

  - `swap`（置换）两对象值，即将两对象的值彼此赋予对方

    ```c++
    namespace std {
        /* std::swap 的典型实现 */
        template <typename T>
        void swap(T& a, T& b) {
            T temp(a);	// copy constructor
            a = b;	// copy assignment
            b = temp;	// copy assignment
        }
    }
    ```

  - **只要类型 T 支持 *copying* （通过 *copy* 构造函数和 *copy assignment* 操作符完成），缺省的`swap`实现代码就会帮你置换类型为 T 的对象，你不需要为此另外再作任何工作**

  - **以指针指向一个对象，内含真正数据**，这种设计的常见表现形式是所谓的 **piml 手法**（pointer to implementation）

    这时 swap 只需要交换两根指针即可，但缺省的 swap 算法不知道这一点：它不仅复制对象，而且还复制对象内维护的指针，缺乏效率。

    解决方案：提供一个特化版，以供使用。

## Logger

- 支持流式日志

  `LOG_INFO << "This is log INFO TEST";`

- Micro

  每一次调用`LOG_XXX`都会生成一个匿名对象`O`，然后调用 `O.stream()` 函数，当连续赋值`<<`结束后，自动调用匿名对象`O`的析构函数，这时根据全局函数`g_output`选择是输出在terminal还是持久化存储

  ```c++
  #define LOG_TRACE \
  	if (muduo::Logger::logLevel() <= muduo::Logger::LogLevel::TRACE) \
  		muduo::Logger(__FILE__, __LINE__, muduo::Logger::LogLevel::TRACE, __func__).stream()
  ...
  ```

  ```c++
  LogStream& stream() { return impl_.stream_; } // 使得连续赋值成为可能
  ```

  ```c++
  Logger::~Logger() {
      impl_.finish();
      const LogStream::Buffer& buf(stream().buffer());
      g_output(buf.data(), buf.length());
      if (impl_.level_ == LogLevel::FATAL) {
          g_flush();
          abort();
      }
  }
  ```

  > **pimpl** 手法
  >
  > ```c++
  > class Logger {
  > public:
  > ...
  > private:
  > class Impl {
  > public:
  >   using LogLevel = Logger::LogLevel;
  > 
  >   /// @brief Constructor
  >   Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
  >   void formatTime();
  >   void finish();
  > 
  >   Timestamp time_; // 时间戳
  >   LogStream stream_;	// 日志流
  >   LogLevel level_;	// 日志等级
  >   int line_;
  >   SourceFile basename_;
  > };
  > Impl impl_;	// 数据对象
  > };
  > ```
  >
  > 

  - **LogStream**

    - 核心数据成员

    ` Buffer buffer_;`

    - 流式日志实现手法：重载 `operatot<<`

    支持类型：bool、short、unsigned short、int、unsigned int、long、unsigned long、long long、unsigned long long、const void\*、float、double、char、const char\*、const unsigned char*、const string&、const StringPiece&、const Buffer&

    其中，`short`、`unsigned short`转换成`int`后格式化到核心数据成员 `Buffer` 中

  - **Buffer**

    - `using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;`

    - `template<int SIZE> class FixedBuffer...`核心数据成员：

      - `char data_[SIZE]`
      - `char* cur_`，指向`data_`

    - 核心函数：

      - 将数据追加到`data_`

        ```c++
        void append(const char* /*restrict*/ buf, size_t len) {
            // FIXME: append partially
            if (implicit_cast<size_t>(avail()) > len) {
                memcpy(cur_, buf, len);
                cur_ += len;
            }
        }
        ```

      - 移动当前指针位置

        ```c++
        void add(size_t len) { cur_ += len; }
        ```

## Mutex

mutex 互斥量，本质上是一把锁。

 *  在访问共享资源前对互斥量进行设置（加锁），访问完成后释放（解锁）互斥量。
 *  互斥量使用 pthread_mutex_t 数据类型表示
 *  调用 pthread_mutex_init 函数进行初始化
 *  对互斥量加锁调用 int pthread_mutex_lock(pthread_mutex_t *mutex)
 *  对互斥量解锁锁调用 int pthread_mutex_unlock(pthread_mutex_t *mutex)

## Atomic

原子类型是封装了一个值的类型.

* 它的访问保证不会导致数据的竞争，并且可以用于在不同的线程之间同步内存访问.

GCC 4.1.2版本之后，对X86或X86_64支持内置原子操作。

* 就是说，不需要引入第三方库（如pthread）的锁保护，即可对1、2、4、8字节的数值或指针类型，进行原子加/减/与/或/异或等操作。

* 根据GCC手册中《Using the GNU Compiler Collection (GCC)》章节内容，将__sync_系列17个函数声明整理简化如下：

  * 将 value 和 *ptr 进行 加/减/或/与/异或/取反，结果更新到*ptr，并返回操作之前*ptr的值

    `type __sync_fetch_and_add (type *ptr, type value, ...)`

    `type __sync_fetch_and_sub (type *ptr, type value, ...)`

    `type __sync_fetch_and_or (type *ptr, type value, ...)`

    `type __sync_fetch_and_and (type *ptr, type value, ...)`

    `type __sync_fetch_and_xor (type *ptr, type value, ...)`

    `type __sync_fetch_and_nand (type *ptr, type value, ...)`

  * 将 value 和 *ptr 进行 加/减/或/与/异或/取反，结果更新到*ptr，并返回操作之后*ptr的值

    `type __sync_add_and_fetch (type *ptr, type value, ...)`

    `type __sync_sub_and_fetch (type *ptr, type value, ...)`

    `type __sync_or_and_fetch (type *ptr, type value, ...)`

    `type __sync_and_and_fetch (type *ptr, type value, ...)`

    `type __sync_xor_and_fetch (type *ptr, type value, ...)`

    `type __sync_nand_and_fetch (type *ptr, type value, ...)`

  * 比较 *ptr 与 oldval 的值，如果两者相等，则将newval更新到*ptr并返回true

    `bool __sync_bool_compare_and_swap (type *ptr, type oldval type newval, ...)`

  * 比较*ptr与oldval的值，如果两者相等，则将newval更新到*ptr并返回操作之前*ptr的值

    `ype __sync_val_compare_and_swap (type *ptr, type oldval type newval, ...)`

  * 发出完整内存栅栏

    `__sync_synchronize (...)`

  * 将value写入*ptr，对*ptr加锁，并返回操作之前*ptr的值。即，try spinlock语义

    `type __sync_lock_test_and_set (type *ptr, type value, ...)`

  * 将0写入到*ptr，并对*ptr解锁。即，unlock spinlock语义

    `void __sync_lock_release (type *ptr, ...)`

 * NOTE：

   - 这个**type**不能乱用(type只能是int, long, long long以及对应的unsigned类型)，同时在用gcc编译的时候要加上选项 -march=i686。

   * 后面的可扩展参数**(…)**用来指出哪些变量需要memory barrier，因为目前gcc实现的是full barrier(类似Linux kernel中的mb()，表示这个操作之前的所有内存操作不会被重排到这个操作之后)，所以可以忽略掉这个参数。

## Condition

Cond(条件变量)，与互斥量一起使用，允许线程以无竞争的方式等待特定的条件发生.

 *  条件本身是由互斥量保护的
 *  线程在改变条件状态之前必须首先锁住互斥量
 *  条件变量使用 `pthread_cond_t `数据类型表示
 *  把 `PTHREAD_COND_INITIALIZER `赋值给静态分配的条件变量进行初始化
 *  使用 `int pthread_cond_init(pthread_cond_t *cond,  const pthread_condattr_t *restrict attr) `对动态分配的条件变量进行初始化
 *  使用` int pthread_cond_destroy(pthread_cond_t *cond) `进行反初始化
 *  使用` int pthread_cond_wait `等待条件变量变为真，如果在给定的时间内条件不能满足，则会生成一个返回错误码的变量
 *  `pthread_cond_signal `至少唤醒一个等待该条件的线程
 *  `pthread_cond_broadcast `唤醒等待该条件的所有线程

## CountDownLatch

- pthread_join: 

  是只要线程 active 就会阻塞，线程结束就会返回，一般用于主线程回收工作线程

- CountDownLatch:

  - 可以保证工作线程的任务执行完毕，主线程再对工作线程进行回收

  * 本质上来说,是一个thread safe的计数器,用于主线程和工作线程的同步

  * 用法：

    *              在初始化时,需要指定主线程需要等待的任务的个数(count), 当工作线程完成 Task Callback后对计数器减1，而主线程通过wait()调用阻塞等待技术器减到0为止.
    *              初始化计数器值为1,在程序结尾将创建一个线程执行CountDown操作并wait()，当程序执行到最后会阻塞直到计数器减为0,这可以保证线程池中的线程都start了线程池对象才完成析构,实现ThreadPool的过程中遇到过

  * 核心函数：

    * `countDwon() `

      对计数器进行原子减一操作

    - `wait()`

      使用条件变量等待计数器减到零，然后notify

## BlockingQueue

- **BlockingQueue是一种数据结构，支持一个线程往里存资源，另一个线程从里取资源**

- 无界阻塞队列，可以容纳非常多的元素，可近似认为是无限容量

- 阻塞队列是一个在队列基础上又支持了两个附加操作的队列:

  -  支持阻塞的**插入**方法：队列满时，队列会阻塞插入元素的线程，直到队列不满。
  -  支持阻塞的**移除**方法：队列空时，获取元素的线程会等待队列变为非空。

- 应用场景

  **阻塞队列常用于生产者和消费者的场景**，生产者是向队列里添加元素的线程，消费者是从队列里取元素的线程。简而言之，阻塞队列是生产者用来存放元素、消费者获取元素的容器。

- 通知模式实现：所谓通知模式，就是当生产者往满的队列里添加元素时会阻塞住生产者，当消费者消费了一个队列中的元素后，会通知生产者当前队列可用。

- **为什么BlockingQueue适合解决生产者消费者问题**

  > 任何有效的生产者-消费者问题解决方案都是通过控制生产者put()方法（生产资源）和消费者take()方法（消费资源）的调用来实现的，一旦你实现了对方法的阻塞控制，那么你将解决该问题。

## AsyncLogger

>  异步日志系统，采用多线程模拟异步IO，采用双缓冲(Double Buffering)机制，运行时日志输出级别可调

### 多生产者-单消费者问题

 *  生产者（前端），要尽量做到低延迟、低CPU开销、无阻塞
 *  消费者（后端），要做到足够大的吞吐量，并占用较少资源

### Double Buffer 基本思路

0. 准备两块 Buffer A B
1. 前端负责往 Buffer A 中填数据（日志信息）
2. 后端负责把 Buffer B 中数据写入文件
3. ×× 当 Buffer A 写满后，交换 A 和 B，让后端将 Buffer A 中的数据写入文件，而前端则往 Buffer B 填入新的日志信息，如此反复 ××

### 优点

1. 双缓冲机制为什么高效？

   - 在大部分的时间中，前台线程和后台线程不会操作同一个缓冲区，这也就意味着前台线程的操作，不需要等待后台线程缓慢的写文件操作(因为不需要锁定临界区)。

    * 后台线程把缓冲区中的日志信息，写入到文件系统中的频率，完全由自己的写入策略来决定，避免了每条新日志信息都触发(唤醒)后端日志线程

      例如，可以根据实际场景，定义一个刷新频率（2秒），只要刷新时间到了，即使缓冲区中的文件很少，也要把他们存储到文件系统中

    *  换言之，前端线程不是将一条条日志信息分别传送给后端线程，而是将多条信息拼成一个大的 buffer传送给后端，相当于是批量处理，减少了线程唤醒的频率，降低开销

2. 如何做到尽可能降低 lock 的时间？

   - 在[大部分的时间中]，前台线程和后台线程不会操作同一个缓冲区。也就是是说，在小部分时间内，它们还是有可能操作同一个缓冲区的。就是：当前台的写入缓冲区 buffer A 被写满了，需要与 buffer B 进行交换的时候，交换的操作，是由后台线程来执行的，具体流程是：

     - 后台线程被唤醒，此时 buffer B 缓冲区是空的，因为在上一次进入睡眠之前，buffer B 中数据已经被写入到文件系统中了

      *  把 buffer A 与 buffer B 进行交换

      *  把 buffer B 中的数据写入到文件系统

      *  开始休眠

    *  交换缓冲区，就是把两个指针变量的值交换一下而已，利用C++语言中的swap操作，效率很高

    *  在执行交换缓冲区的时候，可能会有前台线程写入日志，因此这个步骤需要在 Lock 的状态下执行

    *  ×× 这个双缓冲机制的前后台日志系统，需要锁定的代码仅仅是交换两个缓冲区这个动作，Lock 的时间是极其短暂的！这就是它提高吞吐量的关键所在！ ××

3. C++ stream 风格：

    *  用起来更自然，不必费心保持格式字符串与参数类型的一致性

    *  可以随用随写

    *  类型安全

    *  当输出的日志级别高于语句的日志级别时，打印日志是个空操作，运行时开销接近零

4. 日志文件滚动 rolling：

    *  条件：文件大小、时间（例如，每天零点新建一个日志文件，不论前一个日志文件是否写满）

    *  自动根据文件大小和时间来主动滚动日志文件

    *  不采用文件改名的方法

### 日志文件名

 *  xxx.20220912-012345.hostname.2333.log
 *  第一部分：`xxx`，进程的名字，容易区分是哪个服务程序的日志，[必要时，可加入版本号]
 *  第二部分：`.20220912-012345`，文件的创建时间(精确到秒)，可利用通配符`*.20220912-01*`筛选日志
 *  第三部分：`hostname`，机器名称，便于溯源
 *  第四部分：`2333`,进程ID，如果一个程序一秒之内反复重启，那么每次都会生成不同的日志
 *  第五部分：统一的后缀名`.log`，便于配套脚本的编写
 *  实现输出操作的是一个线程，那么在写入期间，这个线程就需要一直持有缓冲区中的日志数据。

### 可以继续优化的地方

 * 异步日志系统中，使用了一个全局锁，尽管临界区很小，但是如果线程数目较多，锁争用也可能影响性能。

 * 一种解决方法是像 Java 的 ConCurrentHashMap 那样使用多个桶子(bucket)，前端线程写日志的时候根据线程id哈希到不同的 bucket 中，以减少竞争。

   这种解决方案本质上就是提供更多的缓冲区，并且把不同的缓冲区分配给不同的线程(根据线程 id 的哈希值)。

   那些哈希到相同缓冲区的线程，同样是存在争用的情况的，只不过争用的概率被降低了很多。
   

## Singleton

1. DCL(Double Checked Locking)，受到乱序执行的影响，DCL无法保证 Singleton 的线程安全

   > Solution: 利用 pthread_once 可是实现 Singleton lazy-initialization 的线程安全

2. 在多线程环境中，有些事仅需要执行一次，通常初始化应用程序时，可以比较容易将其放在 main 函数中。但当写一个库时，就不能在 main 里初始化了，这时可以使用静态初始化，但使用一次初始化(pthread_once)会比较容易

3. `int pthread_once(pthread_once_t *once_control, void (*init_routine) (void))；`

   功能：本函数使用初值为`PTHREAD_ONCE_INIT`的`once_control`变量保证`init_routine()`函数在本进程执行序列中仅执行一次。

   在多线程环境下，尽管`pthread_once()`调用会出现在多个线程中，但是`init_routine()` 函数只执行一次究竟在哪个线程中执行是不确定的，有内核调度来决定

   Linux Threads使用互斥锁和条件变量保证由`pthread_once()`指定的函数执行且仅执行一次，而`once_control`表示是否执行过。

   如果`once_control`的初值不是`PTHREAD_ONCE_INIT（Linux Threads定义为0）`，pthread_once() 的行为就会不正常。

## Channel

- selectable IO channel
- 负责注册与响应IO事件，但不拥有 file descriptor

## Sockets

- RAIIhandle，封装一个filedescriptor，并在析构时关闭fd

## Poller

> PollPoller 和 EPollPoller 的基类，采用“电平触发(LT, Level Trigger)”语意

> ET 模式在很大程度上降低了一个 epoll 事件被重复触发的次数，因此效率要比 LT 模式高

### LT模式  

- 是 epoll 的默认工作模式，这种情况下 epoll 相当于一个效率较高的 poll
- 读事件触发后，可以按需收取想要的字节数，不用把本次接收到的数据收取干净（即不用循环到 recv 或者 read 函数返回 -1，错误码为 EWOULDBLOCK 或 EAGAIN）
- 不需要写事件一定要及时移除，避免不必要的触发，浪费 CPU 资源
- 可以自由决定每次收取多少字节（对于普通 socket）或何时接收连接（对于侦听 socket），但是可能会导致多次触发

### ET模式

- 当往 epoll 内核事件表中注册一个文件描述符上的 EPOLLET 事件时，epoll 将以 ET 模式来操作该文件描述符，是 epoll 的高效工作模式
- 读事件必须把数据收取干净，因为你不一定有下一次机会再收取数据了，即使有机会，也可能存在上次没读完的数据没有及时处理，造成客户端响应延迟

- 写事件触发后，如果还需要下一次的写事件触发来驱动任务（例如发上次剩余的数据），你需要继续注册一次检测可写事件
- 必须每次都要将数据收完（对于普通 socket）或必须理解调用 accept 接收连接（对于侦听socket），其优点是触发次数少
- ET 使用准则: 只有出现EAGAIN错误才调用epoll_wait

## 压力测试

### ping pong

1. ping pong协议是客户端和服务器都实现echo协议

   当TCP连接建立时,客户端向服务器发送一些数据,服务器会echo回这些
   数据,然后客户端再echo回服务器；这些数据就会像乒乓球一样在客户端和服务器之间来回传送,直到有一方断开连接为止。

2. 这是用来测试吞吐量的常用办法。注意数据是无格式的,双方都是收到多少数据就反射回去多少数据,并不拆包,这与后面的ZeroMQ延迟测试不同
