#include "groupmodel.hpp"
#include "db.h"
#include <iostream>

    // 创建群组
    bool GroupModel::createGroup(Group &group)
    {
        // 1. 组装sql语句
        char sql[1024] = {0};
        cout << "group.getName() = " << group.getName() << endl;
        sprintf(sql, "INSERT INTO AllGroup(groupname, groupdesc) VALUES('%s', '%s')", group.getName().c_str(), group.getDesc().c_str());
        // 2. 连接数据库
        MySQL mysql;
        if (mysql.connect()) {
            if (mysql.update(sql)) {
                group.setId(mysql_insert_id(mysql.getConnection()));
                return true;
            }
        }
        return false;
    }

    // 加入群组
    void GroupModel::addGroup(int userid, int groupid, string role)
    {
        // 1. 组装sql语句
        char sql[1024] = {0};
        sprintf(sql, "INSERT INTO GroupUser(userid, groupid, grouprole) VALUES(%d, %d, '%s')", userid, groupid, role.c_str());
        // 2. 连接数据库
        MySQL mysql;
        if (mysql.connect()) {
            mysql.update(sql);
        }
    }
    // 查询用户所在群组信息
    vector<Group> GroupModel::queryGroups(int userid)
    {
        /* 
        1. 先根据userid在GroupUser表中查询出所有的群id
        2. 再根据群id查询群信息
        */
        char sql[1024] = {0};
        sprintf(sql, "SELECT a.id, a.groupname, a.groupdesc FROM AllGroup a INNER JOIN GroupUser b \
            ON a.id = b.groupid WHERE b.userid = %d", userid);

        vector<Group> groupVec;

        MySQL mysql;
        if (mysql.connect()) {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr) {
                    Group group;
                    group.setId(atoi(row[0]));
                    group.setName(row[1]);
                    group.setDesc(row[2]);
                    groupVec.push_back(group);
                }
                mysql_free_result(res);
                return groupVec;
            }
        }
        return groupVec;
    }
    // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其他成员群发消息
    vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
    {
        // 1. 组装sql语句
        char sql[1024] = {0};
        sprintf(sql, "SELECT userid FROM GroupUser WHERE groupid = %d AND userid != %d", groupid, userid);

        // 2. 连接数据库
        vector<int> idVec;
        MySQL mysql;
        if (mysql.connect()) {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr) {
                    idVec.push_back(atoi(row[0]));
                }
                mysql_free_result(res);
                return idVec;
            }
        }
        return idVec;
    }