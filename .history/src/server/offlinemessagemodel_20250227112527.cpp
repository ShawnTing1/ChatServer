#include "offlinemessagemodel.hpp"

// 存储用户离线消息
    bool OfflineMessageModel::insert(int userid, std::string msg);

    // 删除用户离线消息
    bool OfflineMessageModel::remove(int userid);
    
    // 查询用户离线消息
    vector<std::string> OfflineMessageModel::query(int userid);