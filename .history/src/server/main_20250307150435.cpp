#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

// 处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int sig)
{
    ChatService::instance()->reset();
    exit(0);
}
int main()
{
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6666);
    // 创建 Server 对象之前先创建一个 Socket 并设置 SO_REUSEADDR
    muduo::net::Socket sock(addr.family()); // 这里的 family 是 IP 地址类型
    sock.setReuseAddr(true);  // 启用地址重用

    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}
