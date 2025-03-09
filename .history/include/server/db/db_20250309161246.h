#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>

using namespace std;


// 数据库操作类
class Connection
{
public:
    // 初始化数据库连接
    Connection();
    // 释放数据库连接资源
    ~Connection();
    // 连接数据库
    bool connect(string ip, unsigned short port, string user, string password, string dbname);
    // 更新操作
    bool update(const string &sql);
    // 查询操作
    MYSQL_RES *query(const string &sql);
    // // 获取连接
    // MYSQL *getConnection();
    // 刷新连接的空闲时间
    void refreshAliveTime() {_aliveTime = clock();}; 
    // 获取连接的空闲时间
    clock_t getAliveTime() {return clock() - _aliveTime;}

private:
    MYSQL *_conn;
    clock_t _aliveTime; // 记录进入空闲状态的时间
};

#endif