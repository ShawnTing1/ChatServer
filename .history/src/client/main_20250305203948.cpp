#include "json.hpp"
#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>


using namespace std;
using json = nlohmann::json;

// 记录当前系统登陆的用户信息
User g_currentUser;
// 记录当前登陆用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登陆用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 显示当前登陆成功用户的基本信息
void showCurrentUserData();

// 显示当前登陆成功用户的基本信息
void showCurrentUserData()
{
    cout << "=============================login user=============================" << endl;
    cout << "current login user => id: " << g_currentUser.getId() << " name: " << g_currentUser.getName() \
    << " state: " << g_currentUser.getState() << endl;
    cout << "==============================friend list============================" << endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << "friend id: " << user.getId() << " name: " << user.getName() << " state: " << user.getState() << endl;
        }
    }
    cout << "==============================group list============================" << endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group& group : g_currentUserGroupList)
        {
            cout << "group id: " << group.getId() << " name: " << group.getName() << " desc: " << group.getDesc() << endl;
        }
    }
    cout << "====================================================================" << endl;
}