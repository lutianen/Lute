# Tools 食用指南

## `nc`

netcat 命令，用于设置路由器，短小精干、功能强大，有着“瑞士军刀”的美誉。

1. 能通过 TCP 和 UDP 在网络中读写数据，主要用来快速构建网络连接：

    - 可以让它以服务器的方式运行，监听某个端口并接收客户连接

    - 也可以使之以客户端方式运行，向服务器发起连接并收发数据

2. 语法：

```bash
nc [-hlnruz][-g<网关...>][-G<指向器数目>][-i<延迟秒数>][-o<输出文件>][-p<通信端口>][-s<来源位址>][-v...][-w<超时秒数>][主机名称][通信端口...]
```

3. 常用命令包括：

    - `-i`：设置数据包传送的时间间隔
    -  `-v`：显示指令执行过程
    -  `-l`：以服务器方式运行，监听指定的端口。nc默认以客户端方式运行
    -  `-k`：重复接受并处理某个端口上的所有连接，必须和 `-l` 选项配合使用
    -  `-n`：使用 IP 地址表示主机，而不是主机名；使用数字表示端口号，而不是服务名称
    
    - `-p`：当 nc 命令以客户端方式运行时，强制其使用指定的端口号
    -  `-s`：设置本地主机发送的数据包的IP地址
    -  `-C`：将 `CR` 和 `LF` 两个字符作为行结束符
    -  `-U`：使用 UNIX 本地域协议通信
    -  `-u`：使用 UDP 协议；nc 默认使用 TCP 协议
    -  `-w`：如果 nc 客户端在指定时间内未检测到任何输入，则退出
    -  `-X`：当 nv 客户端和代理服务器通信时，该选项指定他们之间使用的通信协议；目前支持的代理协议包括："4"(SOCKS v.4)，"5"(SOCKS v.5) 和"connect"(HTTPS proxy)；默认使用SOCKS v.5
    -  `-x`：指定目标代理服务器的 IP 地址和端口号
    -  `-z`：扫描目标机器上的某个或某些服务是否开启（端口扫描）
    -  `-g`：<网关> # 设置路由器跃程通信网关，最多可设置8个。
    -  `-h `：在线帮助
    -  `-o`：<输出文件> # 指定文件名称，把往来传输的数据以16进制字码倾倒成该文件保存。
    -  `-r`： 乱数指定本地与远端主机的通信端口。

4. 实例

    1. **客户端**

        ```sh
        nc -n 127.0.0.1 5836
        ```

        

    2. **TCP端口扫描**

        ```sh
        # 扫描 10.196.124.133 的端口 范围是 1-100
        nc -v -z -w2 10.196.124.133 1-100
        ```

    3. **文件传输**

        ```sh
        # 接收方 提前设置监听端口与要接收的文件名（文件名可自定义）：
        nc -lp 8888 > node.tar.gz
        
        # 传输方 发文件：
        nc -nv 192.168.75.121 8888  < node_exporter-1.3.1.linux-amd64.tar.gz
        # ⚠️ 注意：192.168.75.121是接收方的ip地址。
        
        # 如果希望文件传输结束后自动退出，可以使用下面的命令：
        nc -lp 8888 > node.tar.gz
        nc -nv 192.168.75.121 8888 -i 1 < node_exporter-1.3.1.linux-amd64.tar.gz
        # ⚠️ 注意：-i 表示闲置超时时间 
        ```

## Google Benchmark

专业的性能测试框架，只需要将要测试的代码放入`for (auto _ : bm)` 循环里即可，框架会自动决定要循环多少次，保证结果是准确的，同时不浪费太多时间。

**CMakeLists.txt 导入**

```cmake
...
find_package(benchmark REQUIRED)
target_link_libraries(test PUBLIC benchmark::benchmark)
...
```

**测试代码**

```c++
#include <iostream>
#include <vector>
#include <cmath>
#include <benchmark/benchmark.h>

constexpr size_t n = 1 << 27;
std::vector<float> a(n);

void BM_for(benchmark::State &bm) {
    for (auto _ : bm) {
        // 待测试代码
        for (size_t i = 0; i < a.size(); ++i) {
            a[i] = std::sin(i);
        }
    }
}
BENCHMARK(BM_for);

BENCHMARK_MAIN();
```

框架中`BENCHMARK_MAIN()`自动生成一个 `main` 函数，从而生成一个可执行文件以供运行。

```bash
# 执行编译生成的可执行文件
./build/bin/test
```

运行会得到的测试结果打印在终端上。

> **命令行参数**

还卡一使用某些命令行参数来控制测试的输出格式为 `csv` 等，可调用 `--help` 查看更多。

```bash
./build/bin/test --benchmark_format=csv
```





