![](https://raw.githubusercontent.com/lutianen/PicBed/main/202306302231793.png)

**Lute is a event-driven network library in Linux. This project uses the epoll I/O event notification mechanism, and it supports GET/POST requests for HTTP protocol.**

## Deployment

1. CMake

    ```bash
    # CMake
    cmake -B build
    cmake --build build
    
    # Usage: /home/lux/githubWorkplace/Lute/build/bin/httpServer serverName staticSrcPrefix serverPort numThreads [IPofMySQLServer[default: 127.0.0.1] PortofMySQLServer[default: 3306] UsernameofMySQLServer[default: lutianen] PasswordofMySQLServer[default: lutianen] DatabaseofMySQLServer[default: user]]
    ./build/app/http/httpServer LuxPolaris /home/lux/Lux/app/HTML 5836 8 192.168.1.108 3306 lutianen lutianen LuxDatabase
    ```

2. Make

    ```bash
    # Make
    
    # optional
    make clean
    make release # make debug
    
    # Need to modify Makefile:
    # Usage: /home/lux/githubWorkplace/Lute/build/bin/httpServer serverName staticSrcPrefix serverPort numThreads [IPofMySQLServer[default: 127.0.0.1] PortofMySQLServer[default: 3306] UsernameofMySQLServer[default: lutianen] PasswordofMySQLServer[default: lutianen] DatabaseofMySQLServer[default: user]]
    make run
    ```

## 特点



## 附录

- [常用工具介绍]()

- [GoogleTest 使用指南]()

- [核心类介绍]()

## 压力测试

### Webbench
