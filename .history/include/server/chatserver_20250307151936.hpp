#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>


#include <muduo/base/Atomic.h>
#include <muduo/base/Types.h>
#include <muduo/net/TcpConnection.h>

using namespace muduo;
using namespace muduo::net;

// 聊天服务器类
class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name, Option option = kNoReusePort);
    // 启动服务
    void start();

private:
    // 新连接回调
    void onConnection(const TcpConnectionPtr &conn);
    // 消息到达回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

    TcpServer _server; // 组合的muduo库，实现服务器功能的类对象
    EventLoop *_loop;  // 指向事件循环对象的指针
};

#endif