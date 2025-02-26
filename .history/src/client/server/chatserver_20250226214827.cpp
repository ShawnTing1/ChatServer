#include "chatserver.hpp"

#include <functional>

using namespace std;
using namespace placeholders;

// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name)
    : _server(loop, listenAddr, name), _loop(loop)
{
    // 注册连接回调函数
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    // 注册消息回调函数
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    // 设置线程数量
    _server.setThreadNum(4);
}


// 启动服务
void ChatServer::start()
{
    _server.start();
}


// 连接回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if(!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 消息回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{

}
