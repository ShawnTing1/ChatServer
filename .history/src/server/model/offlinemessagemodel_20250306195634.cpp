#include "offlinemessagemodel.hpp"
#include "db.h"
#include "json.hpp"

#include <iostream>
using json = nlohmann::json;

// 存储用户离线消息
// void OfflineMsgModel::insert(int userid, string msg)
// {
//     // 1. 组装sql语句
//     char sql[1024] = {0};
//     sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());
//     // 2. 执行sql语句
//     MySQL mysql;
//     if (mysql.connect())
//     {
//         mysql.update(sql);
//     }
// }

void OfflineMsgModel::insert(int userid, string msg)
{
    cout << "OfflineMsgModel::insert()" << msg << endl;
    MySQL mysql;
    if (mysql.connect())
    {
        // 1. 查询当前用户的离线消息
        char sql[1024] = {0};
        sprintf(sql, "SELECT message FROM OfflineMessage WHERE userid = %d", userid);
        MYSQL_RES *res = mysql.query(sql);

        json messages = json::array();  // 使用 nlohmann::json 存储离线消息

        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row && row[0]) {
                try {
                    messages = json::parse(row[0]);  // 解析旧的消息
                } catch (exception& e) {
                    cout << "JSON parse error: " << e.what() << endl;
                }
            }
            mysql_free_result(res);
        }

        // 2. 追加新消息
        json newMessage = json::parse(msg);  // 解析新的消息 JSON
        messages.push_back(newMessage);      // 追加到 JSON 数组中

        // 3. 更新用户的离线消息
        string updatedMessages = messages.dump();  // 转换为字符串存储
        sprintf(sql, "REPLACE INTO OfflineMessage (userid, message) VALUES (%d, '%s')", userid, updatedMessages.c_str());
        mysql.update(sql);
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