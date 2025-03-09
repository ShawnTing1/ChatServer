/*
实现连接池功能模块
*/

#include <iostream>
#include <queue>
#include <mutex>
#include <string>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>
#include <functional>

#include "db.h"

using namespace std;

class ConnectionPool
{
public:
    // 获取连接池对象实例
    static ConnectionPool* GetInstance();
    shared_ptr<Connection> getConnection(); // 给外部提供接口，从连接池中获取一个空闲连接  [消费者]
private:
    ConnectionPool(); // 单例模式，构造函数私有化
    static ConnectionPool* _connPool;

    // 从配置文件中加载配置项
    bool loadConfigFile();

    // 运行在独立线程中，生产者线程函数，生产新连接  [生产者]
    void produceConnectionTask();

    // 运行在独立线程中，检测空闲连接的回收线程函数
    void scannerConnectionTask();
    
    string _ip; // mysql的ip地址
    unsigned short _port; //mysql的端口号 3306
    string _username; // mysql登陆用户名
    string _password; //mysql登陆密码
    string _dbname;
    int _initSize; // 连接池的初始连接量
    int _maxSize; // 连接池的最大连接量
    int _maxIdleTime; // 连接池的最大空闲时间
    int _connectionTimeout; // 连接池获取连接的超时时间

    queue<Connection*> _connectionQueue; // 存储mysql连接的队列
    mutex _queueMutex; // 维护连接队列的线程安全互斥锁
    atomic_int _connectionCnt; // 记录连接所创建的连接数
    condition_variable _cv; // 生产者消费者模型的条件变量
};