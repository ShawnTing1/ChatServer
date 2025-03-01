#include "groupmodel.hpp"

    // 创建群组
    bool GroupModel::createGroup(Group &group);
    // 加入群组
    void GroupModel::addGroup(int userid, int groupid, string role);
    // 查询用户所在群组信息
    vector<Group> GroupModel::queryGroups(int userid);
    // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
    vector<int> GroupModel::queryGroupUsers(int userid, int groupid);