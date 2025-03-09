#include "db.h"
#include "public.hpp"
#include <muduo/base/Logging.h>

using namespace std;

// 服务器配置信息
// static string server = "127.0.0.1";
// static string user = "root";
// static string password = "12345678";
// static string dbname = "chat";

// 初始化数据库连接
Connection::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 释放数据库连接资源
Connection::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}
// 连接数据库
bool Connection::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // 设置字符集
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_ERROR << "connect mysql fail!";
    }

    return p;
}
// 更新操作
bool Connection::update(const string &sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR << __FILE__ << ":" << __LINE__ << ":query error:" << sql;
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES *Connection::query(const string &sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_ERROR << __FILE__ << ":" << __LINE__ << ":query error:" << sql;
        return nullptr;
    }
    return mysql_use_result(_conn);
}


// MYSQL* Connection::getConnection()
// {
//     return _conn;
// }