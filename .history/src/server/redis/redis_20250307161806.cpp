#include "redis.hpp"
#include <iostream>

using namespace std;

Redis::Redis() : _publish_context(nullptr), _subscribe_context(nullptr)
{
}

Redis::~Redis() 
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }

    if (_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }
}

// 连接redis服务器
bool Redis::connect()
{
    // 负责发布消息的上下文连接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr) {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 负责订阅消息的上下文连接
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (_subscribe_context == nullptr) {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 在独立线程中，监听通道消息，有消息则调用回调函数，通知service层
    thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;
    return true;
}

// 向redis指定的通道channel发布消息
bool publish(int channel, string message);

// 向redis指定的通道subscribe订阅消息
bool subscribe(int channel);

// 向redis指定的通道unsubscribe取消订阅消息
bool unsubscribe(int channel);

// 在独立线程中接收订阅通道中的消息
void observer_channel_message();

// 初始化向service层上报消息的回调对象
void init_notify_handler(function<void(int, string)> fn);