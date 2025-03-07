#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <functional>
#include <muduo/base/Logging.h>

using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name)
    : _server(loop, listenAddr, name), _loop(loop)
{
    // 设置端口重用
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    ::close(sockfd);

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
        // 用户断开连接的业务逻辑
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 消息回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    // 数据反序列化
    json js = json::parse(buf);
    // 达到目的：完全解耦网络模块代码和业务模块代码
    // 通过js["msgid"]获取消息类型 -》 业务handler -》 conn js time
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);

}
