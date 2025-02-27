#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <functional>
#include <unordered_map>    
using namespace std;
using namespace muduo;
using namespace muduo::net;

// 表示处理消息的事件回调方法类型
using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;
// 等于C语言中的typedef void (*MsgHandler)(const TcpConnectionPtr& conn, json& js, Timestamp time);

// 聊天服务器业务层类
class ChatService {
public:

private:
};

#endif