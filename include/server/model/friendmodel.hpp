#ifndef FRIENDMODEL_HPP
#define FRIENDMODEL_HPP

#include <vector>
#include "user.hpp"
using namespace std;

// 维护好友信息的操作接口
class FriendModel
{
public:
    // 添加好友关系
    void insert(int userid, int friendid);

    // 返回用户的好友列表 friendid
    vector<User> query(int userid);
private:
};

#endif