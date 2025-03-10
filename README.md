# ChatServer

> 本项目在 Linux 环境下基于 C++ 的 Muduo 网络库，实现了一个高性能的集群聊天服务器。服务器支持新用户注册、用户登录、添加好友、创建群组、点对点通信、群组聊天以及离线消息存储等功能。通过 Nginx 实现 TCP 负载均衡，并利用 Redis 的发布订阅机制作为中间件，完成跨服务器消息同步，同时采用 MySQL 连接池优化数据库访问性能。

## 项目特点

- **高效网络模块**：基于 Muduo 网络库开发，实现高效的网络通信（也可采用 Epoll + Reactor 模型实现 TCP 服务器）。
- **数据序列化**：使用第三方 JSON 库实现数据的序列化和反序列化。
- **负载均衡**：借助 Nginx 的 TCP 负载均衡功能，将客户端请求分发到多个服务器，从而提升并发处理能力。
- **跨服务器通信**：采用 Redis 的发布-订阅消息队列，实现集群内各服务器间通信。
- **数据持久化**：封装 MySQL 接口，将用户数据存储到磁盘，确保数据的持久化。
- **CMake 构建**：使用 CMake 管理项目构建流程。
- **单例模式**：在 ChatService 模块中采用饿汉式单例模式，确保在进程启动时创建对象，并保证线程安全。
- **数据库连接池**：通过queue队列容器，生产者-消费者线程模型，线程互斥等实现了连接池，减少访问数据库的性能消耗。

## 功能

### 客户端

- 注册
- 登陆
- 添加好友，好友单聊
- 创建群组，加入群组，群组聊天
- 离线信息存储

### 服务端

- TCP负载均衡
- 跨服务器通信

### 数据库

- 初始连接量（initSize）
- 最大连接量（maxSize）
- 最大空闲时间（maxIdleTime）
- 连接超时时间（connectionTimeout）

## 必要环境

> Linux系统，Boost&Muduo，MySQL，Nginx，Redis，hiredis。

- **操作系统**：Linux (Ubuntu 18.04 或 Ubuntu 20.04)

- **依赖库**：

  - 安装 `Boost` 和 Muduo 网络库  
    [CSDN 博客：C++ Muduo 网络库源码编译安装](https://blog.csdn.net/qq_58158950/article/details/135669038?ops_request_misc=%7B%22request%5Fid%22%3A%22170850414816777224454509%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fblog.%22%7D&request_id=170850414816777224454509&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-2-135669038-null-null.nonecase&utm_term=muduo&spm=1018.2226.3001.4450)

  - 安装 MySQL  
    [CSDN 博客：在 Ubuntu 安装 MySQL 5.7（图文详解）](https://blog.csdn.net/qq_58158950/article/details/135667062?ops_request_misc=%7B%22request%5Fid%22%3A%22170850438216800185861806%22%2C%22scm%22%3A%2220140713.130102334.pc%5Fblog.%22%7D&request_id=170850438216800185861806&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_ecpm_v1~rank_v31_ecpm-4-135667062-null-null.nonecase&utm_term=mysql&spm=1018.2226.3001.4450)

  - 安装 Nginx  

    ```bash
    # 激活 TCP 负载均衡模块（需加入 --with-stream 参数）
    ./configure --with-stream
    make && make install
    ```

    配置 TCP 负载均衡，编辑 `conf/nginx.conf` 文件：

    ```bash
    stream {
        upstream MyServer {
            server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
            server 127.0.0.1:6001 weight=1 max_fails=3 fail_timeout=30s;
        }
    
        server {
            proxy_connect_timeout 1s;
            listen 8000;
            proxy_pass MyServer;
            tcp_nodelay on;
        }
    }
    ```

    重新加载配置：

    ```bash
    nginx -s reload
    ```

  - 安装 Redis  

    ```bash
    sudo apt-get install redis-server
    ```

  - 安装hiredis（redis对应的c++客户端编程语言）

    克隆源码

    ```bash
    git clone https://github.com/redis/hiredis
    ```

    进入源码文件并编译安装

    ```bash
    cd hiredis
    ```

    ```bash
    make
    ```

    ```bash
    sudo make install
    ```

    拷贝生成的动态库到/usr/local/lib目录下

    ```bash
    sudo ldconfig /usr/local/lib
    ```

## 构建项目

- **创建数据库**  

```shell
mysql -u root -p your_password
create database chat;
source chat.sql
```

- **构建项目**  

```shell
bash build.sh
```

## 启动服务端

```shell
cd ./bin
./ChatServer 6666
```

## 启动客户端

```shell
./ChatClient 127.0.0.1 8000
```

## 运行测试

> **说明**：本项目支持集群服务器功能及跨服务器通信，测试时既可使用单台服务器，也可同时启动多台服务器。本次运行测试打开两个服务器，打开一个客户端，客户端通过8000端口连接Nginx，Nginx通过轮询方式转发给6000和6001端口的服务器。

### 运行多服务器测试

```bash
./ChatServer 127.0.0.1 6000
./ChatServer 127.0.0.1 6001
```

### 运行客户端

```bash
./ChatClient 127.0.0.1 8000
```

运行结果：

![服务器负载均衡](https://s3.bmp.ovh/imgs/2025/03/08/60c9532f392d647a.png)

首页：

```bash
=======================Welcome to Chat System======================
1. login
2. register
3. quit
====================================================================
please select menu: 
```



## 功能测试

> 测试功能：注册，登陆，点对点聊天，创建群组，加入群组，群组聊天。

### 注册

```bash
please select menu: 
2
please input your name: Shuo
please input your password: 123456
Shuo register success, userid is 10 please remember it!
```

![注册成功](https://s3.bmp.ovh/imgs/2025/03/08/62a93fc05037bd34.png)

### 登录

```bash
please select menu: 
1
please input your id: 1
please input your password: 123456
1 login success!
```

![登录成功](https://s3.bmp.ovh/imgs/2025/03/08/8741577f36de07ac.png)

### 添加好友

```bash
# 格式: addfriend:friendid
addfriend:10
```

![添加好友](https://s3.bmp.ovh/imgs/2025/03/08/17429da3dec31515.png)

### 点对点聊天

```bash
# 格式: chat:friendid:message
chat:2:hello # Client1
2025-03-08 11:43:49 [1] Shawn said: hello #Client2
```

![私聊消息](https://s3.bmp.ovh/imgs/2025/03/08/8fc9ee2cc4338bb4.png)

### 创建群组

```bash
#格式: creategroup: groupname: groupdesc
creategroup:myfamily:The Group Of My Family
```

![](https://s3.bmp.ovh/imgs/2025/03/08/e8e68b3616c7d24f.png)

### 加入群组

```bash
#格式: addgroup: groupid
addgroup:2
```

![加入群组](https://s3.bmp.ovh/imgs/2025/03/08/2795b1807a09de02.png)

### 群组聊天

```bash
#格式: groupchat: groupid: message
groupchat:2:hello everyone!!!! #Client1
Group Message: 2025-03-08 11:57:55 [2] Shawn said: hello everyone!!!! #Client2
Group Message: 2025-03-08 11:57:55 [2] Shawn said: hello everyone!!!! #Client3
```

![群聊消息](https://s3.bmp.ovh/imgs/2025/03/08/f2268753768959d0.png)