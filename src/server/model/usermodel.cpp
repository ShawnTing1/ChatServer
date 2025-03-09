#include "usermodel.hpp"
#include "db.h"
#include "connectionPool.h"
#include <iostream>

using namespace std;

bool UserModel::insert(User& user) {
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
        user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    // 2.连接数据库
    ConnectionPool* cp = ConnectionPool::GetInstance();
    shared_ptr<Connection> sp = cp->getConnection();
    if (sp) {
        if (sp->update(sql)) {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(sp->getConnection()));
            return true;
        }
    }
    
    return false;
}


User UserModel::query(int id) {
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = %d", id);
    // 2.连接数据库
    ConnectionPool *cp = ConnectionPool::GetInstance();
    shared_ptr<Connection> sp = cp->getConnection();
    if (sp) {
        MYSQL_RES* res = sp->query(sql);
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    
    return User();
}



bool UserModel::updateState(User user) {
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where id = %d",
        user.getState().c_str(), user.getId());
    // 2.连接数据库
    ConnectionPool *cp = ConnectionPool::GetInstance();
    shared_ptr<Connection> sp = cp->getConnection();
    if (sp) {
        if (sp->update(sql)) {
            return true;
        }
    }
    
    return false;
}


void UserModel::resetState() {
    // 1.组装sql语句
    char sql[1024] = "update User set state = 'offline' where state = 'online'";
    // 2.连接数据库
    ConnectionPool *cp = ConnectionPool::GetInstance();
    shared_ptr<Connection> sp = cp->getConnection();
    if (sp) {
        sp->update(sql);
    }
}