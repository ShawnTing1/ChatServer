#include "connectionPool.h"
#include "public.hpp"


// 提前创建单例实例（饿汉模式），程序启动时即初始化，线程安全
ConnectionPool* ConnectionPool::_connPool = new ConnectionPool();

// 获取单例对象
ConnectionPool* ConnectionPool::GetInstance()
{
    return _connPool;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE* pf = fopen("../src/mysql.ini", "r");
    if (pf == nullptr) {
        LOG("mysql.ini file is not exist!");
        return false;
    }

    while (!feof(pf)) {
        char line[1024] = { 0 };
        fgets(line, 1024, pf);
        string str = line;
        int idx = str.find("=", 0);
        if (idx == -1) { // 无效配置项
            continue;
        }

        // password=12345678
        int endIdx = str.find("\n", idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx + 1, endIdx - idx - 1);

        if (key == "ip") {
            _ip = value;
        } else if (key == "port") {
            _port = atoi(value.c_str()); // atoi:string -> int  c_str():string转char*
        } else if (key == "username") {
            _username = value;
        } else if (key == "password") {
            _password = value;
        } else if (key == "dbname") {
            _dbname = value;
        } else if (key == "initSize") {
            _initSize = atoi(value.c_str());
        } else if (key == "maxSize") {
            _maxSize = atoi(value.c_str());
        } else if (key == "maxIdleTime") {
            _maxIdleTime = atoi(value.c_str());
        } else if (key == "connectionTimeout") {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

// 连接池的构造
ConnectionPool::ConnectionPool()
{
    // 加载配置文件了
    if (!loadConfigFile()) {
        return;
    }

    // 创建初始数量的连接
    for (int i = 0; i < _initSize; i++) {
        Connection* conn = new Connection();
        conn->connect(_ip, _port, _username, _password, _dbname);
        conn->refreshAliveTime(); // 刷新连接的空闲时间
        _connectionQueue.push(conn);
        _connectionCnt++;
    }

    // 启动一个新的线程，作为连接的生产者  linux thread -> pthread_create
    thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach(); // 与主线程分离，主线程结束，子线程也结束

    // 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行回收
    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

// 运行在独立线程中，生产者线程函数，生产新连接
void ConnectionPool::produceConnectionTask()
{
    for (;;) {
        unique_lock<mutex> lock(_queueMutex);  // 加锁：用于保护共享数据，防止多线程竞争 解锁时间：离开作用域自动解锁
        while (!_connectionQueue.empty()) {
            _cv.wait(lock); // 队列不空，此处生产线程进入等待状态
        }

        if (_connectionCnt < _maxSize) {
            Connection* conn = new Connection();
            conn->connect(_ip, _port, _username, _password, _dbname);
            conn->refreshAliveTime(); // 刷新连接的空闲时间
            _connectionQueue.push(conn);
            _connectionCnt++;
        }
        _cv.notify_all(); // 通知消费者线程，队列不空
    }
}

void ConnectionPool::scannerConnectionTask()
{
    for (;;) {
        // 通过sleep_for模拟定时任务
        this_thread::sleep_for(chrono::seconds(_maxIdleTime)); // 线程休眠
        
        // 扫描整个队列，释放多余的连接
        unique_lock<mutex> lock(_queueMutex);
        while (_connectionCnt > _initSize) {
            Connection* conn = _connectionQueue.front();
            if (conn->getAliveTime() >= (_maxIdleTime * 1000)) {
                _connectionQueue.pop();
                _connectionCnt--;
                delete conn;
            } else {
                break; // 队列头部连接未超过最大空闲时间，后面的连接也不会超过，直接退出
            }
        }
    }
}


shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);
    while (_connectionQueue.empty()) {
        if (cv_status::timeout == _cv.wait_for(lock, chrono::seconds(_connectionTimeout))) {
            LOG("get connection timeout, get connection failed!");
            return nullptr;
        }
    }

    // shared_ptr智能指针析构时，会自动调用Connection的析构函数，释放连接，connection被close。
    // 这里需要自定义shared_ptr的析构函数，不要调用Connection的析构函数，否则会释放连接。

    shared_ptr<Connection> sp(_connectionQueue.front(), 
        [&](Connection* conn) {
            // 在服务器应用线程中调用，要考虑队列的线程安全操作
            unique_lock<mutex> lock(_queueMutex);
            conn->refreshAliveTime(); // 刷新连接的空闲时间
            _connectionQueue.push(conn);
    });
    _connectionQueue.pop();
    _cv.notify_all(); // 消费完连接，通知生产者线程，检查队列是否为空，生产新连接
    return sp;
}