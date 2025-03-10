#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <functional>
#include <unordered_map>    

#include "json.hpp"


using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;


// 表示处理消息的事件回调方法类型
using MsgHandler = function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;
// 等于C语言中的typedef void (*MsgHandler)(const TcpConnectionPtr& conn, json& js, Timestamp time);

// 聊天服务器业务层类
class ChatService {
public:
    // 获取单例对象的接口函数
    static ChatService* instance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
private:
    ChatService();

    // 存储消息id和其对应的业务处理方法 
    unordered_map<int, MsgHandler> _msgHandlerMap;
};

#endif