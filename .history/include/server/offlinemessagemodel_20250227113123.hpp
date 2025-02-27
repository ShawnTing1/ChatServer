#ifndef OFFLINEMESSAGEMODEL_HPP
#define OFFLINEMESSAGEMODEL_HPP

#include <vector>
#include <string>

using namespace std;

class OfflineMsgModel
{
public:
    // 存储用户离线消息
    void insert(int userid, string msg);

    // 删除用户离线消息
    void remove(int userid);

    // 查询用户离线消息
    vector<std::string> query(int userid);
};

#endif