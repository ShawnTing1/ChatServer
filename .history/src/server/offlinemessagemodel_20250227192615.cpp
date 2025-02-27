#include "offlinemessagemodel.hpp"
#include "db.h"

void OfflineMsgModel::insert(int userid, string msg)
{
    MySQL mysql;
    if (mysql.connect())
    {
        // 1. 查询当前用户的离线消息
        char sql[1024] = {0};
        sprintf(sql, "SELECT message FROM OfflineMessage WHERE userid = %d", userid);
        string existingMessages = mysql.query(sql);  // 假设 mysql.query() 可以返回查询结果

        // 2. 将新消息追加到现有消息
        string newMessage = msg;
        if (!existingMessages.empty()) {
            // 如果用户已经有离线消息，合并新消息
            existingMessages = existingMessages.substr(0, existingMessages.length() - 1); // 去除最后的花括号
            existingMessages += "," + newMessage + "}"; // 追加新消息并结束JSON
        } else {
            // 如果用户没有离线消息，初始化 JSON 格式
            existingMessages = "{\"messages\": [" + newMessage + "]}";
        }

        // 3. 更新用户的离线消息
        sprintf(sql, "REPLACE INTO OfflineMessage (userid, message) VALUES (%d, '%s')", userid, existingMessages.c_str());
        mysql.update(sql);  // 执行更新操作
    }
}

// 删除用户离线消息
void OfflineMsgModel::remove(int userid)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);
    // 2. 执行sql语句
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户离线消息
vector<std::string> OfflineMsgModel::query(int userid)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid=%d", userid);
    // 2. 执行sql语句
    vector<string> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;         
        }
    }
    return vec;
}