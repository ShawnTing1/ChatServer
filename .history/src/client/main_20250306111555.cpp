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

// 接收服务器消息线程
void readTaskHandler(int clientfd);
// 获取系统时间
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int clientfd);

// 聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char** argv)
{
    if (argc > 3) {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6666" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的服务器端的ip和端口
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建客户端socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
        cerr << "create client socket error!" << endl;
        exit(-1);
    }

    // 填写client需要连接的server的ip和port
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // 连接服务器
    if (connect(clientfd, (struct sockaddr*)&server, sizeof(server)) == -1) {
        cerr << "connect server error!" << endl;
        close(clientfd);
        exit(-1);
    }

    // main线程用户接收用户输入，负责发送数据
    for(;;) {
        // 显示页面菜单 登陆，注册，退出
        cout << "=======================Welcome to Chat System======================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "====================================================================" << endl;
        cout << "please select menu: " << endl;
        int choice = 0;
        cin >> choice;
        cin.get(); // 读取缓冲区的回车符

        switch (choice) {
            case 1: // login
            {
                int id = 0;
                char pwd[50] = {0};
                cout << "please input your id: ";
                cin >> id;
                cin.get(); // 读取缓冲区的回车符
                cout << "please input your password: ";
                cin.getline(pwd, 50);

                // 封装登陆数据
                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();

                // 发送登陆数据到服务器
                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1) {
                    cerr << "send login message error!" << endl;
                } else {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if (len == -1) {
                        cerr << "recv login response error!" << endl;
                    } else {
                        json response_js = json::parse(buffer);
                        if (0 != response_js["errno"].get<int>()) {
                            cerr << "login error, please check your id and password!" << endl;
                        } else { // 登陆成功
                            cout << id << " login success!" << endl;
                            // 记录当前登陆用户的id，name
                            g_currentUser.setId(id);  
                            g_currentUser.setName(response_js["name"]);
                            // 记录当前登陆用户的好友列表信息
                            if (response_js.contains("friends")) {
                                vector<string> vec = response_js["friends"];
                                for (string& str : vec) {
                                    json js = json::parse(str);
                                    User user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    g_currentUserFriendList.push_back(user);
                                }
                            }

                            // 记录当前登陆用户的群组列表信息
                            if (response_js.contains("groups")) {
                                vector<string> vec1 = response_js["groups"];
                                for (string& groupstr : vec1) {
                                    json grpjs = json::parse(groupstr);
                                    Group group;
                                    group.setId(grpjs["id"]);
                                    group.setName(grpjs["groupname"]);
                                    group.setDesc(grpjs["groupdesc"]);

                                    vector<string> vec2 = grpjs["users"];
                                    for (string& userstr : vec2) {
                                        json usrjs = json::parse(userstr);
                                        GroupUser user;
                                        user.setId(usrjs["id"].get<int>());
                                        user.setName(usrjs["name"]);
                                        user.setState(usrjs["state"]);
                                        user.setRole(usrjs["role"]);
                                        group.getUsers().push_back(user);
                                    }
                                    g_currentUserGroupList.push_back(group);
                                }
                            }
                            
                            // 显示当前登陆成功用户的基本信息
                            showCurrentUserData();

                            // 显示当前用户的离线消息，个人聊天信息或者群聊信息
                            if (response_js.contains("offlinemsg")) {
                                for (json item : response_js["offlinemsg"]) {
                                    string time = item["time"];
                                    string name = item["name"];
                                    string msg = item["msg"];
                                    cout << name << " " << time << " said: " << msg << endl;
                                }

                            }

                            // 登陆成功，启动接收线程
                            thread readTask(readTaskHandler, clientfd);
                            readTask.detach();
                            // 进入主聊天页面
                            mainMenu(clientfd);
                        }
                    }
                }
            }
            break;
            case 2: // register
            {
                char name[50] = {0};
                char pwd[50] = {0};
                cout << "please input your name: ";
                cin.getline(name, 50);
                cout << "please input your password: ";
                cin.getline(pwd, 50);

                // 封装注册数据
                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();

                // 发送注册数据到服务器
                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1) {
                    cerr << "send register message error!" << endl;
                } else {
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if (len == -1) {
                        cerr << "recv register response error!" << endl;
                    } else {
                        json response_js = json::parse(buffer);
                        if (0 != response_js["errno"].get<int>()) {
                            cerr << name << " is already exist, register error!" << endl;
                        } else {
                            cout << name << " register success, userid is " << response_js["id"].get<int>() 
                            << " please remember it!" << endl;
                        }
                    }
                }
            }
            break;
            case 3: // quit
            {
                close(clientfd);
                exit(0);
            }
            break;
            default:
            {
                cerr << "invalid input!" << endl;
            }
            break;
        }
    }
}


void readTaskHandler(int clientfd)
{
    cout << "01" << endl;
    for(;;) {
        cout << "02" << endl;
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        cout << "03" << endl;
        if (len == -1 || len == 0) {
            cerr << "recv server data error!" << endl;
            close(clientfd);
            exit(-1);
        }
        // 解析服务器发送的json数据
        json js = json::parse(buffer);
        cout << "04" << endl;
        cout << "recv server data: " << js.dump() << endl;
        if (ONE_CHAT_MSG == js["msgid"].get<int>()) {
            cout << "05" << endl;
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>() << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
    }
}



// 显示当前登陆成功用户的基本信息
void showCurrentUserData()
{
    cout << "=============================login user=============================" << endl;
    cout << "current login user => id: " << g_currentUser.getId() << " name: " << g_currentUser.getName() << endl;
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
            cout << "group users list: " << endl;
            for (GroupUser& user : group.getUsers())
            {
                cout << "group user id: " << user.getId() << " name: " << user.getName() << " state: " << user.getState()
                << "role: " << user.getRole() << endl;
            }
        }
    }
    cout << "====================================================================" << endl;
}

// 获取系统时间
string getCurrentTime()
{
    auto tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm* ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", 
        (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
        (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return date;
}


// help command handler
void help(int = 0, string = "");
// chat command handler
void chat(int, string);
// addfriend command handler
void addFriend(int, string);
// creategroup command handler
void createGroup(int, string);
// addgroup command handler
void addGroup(int, string);
// groupchat command handler
void groupChat(int, string);
// loginout command handler
void loginout(int, string);


// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "show command list, format: help"},
    {"chat", "chat with friend, format: chat: friendid: message"},
    {"addfriend", "add friend, format: addfriend: friendid"},
    {"creategroup", "create group, format: creategroup: groupname: groupdesc"},
    {"addgroup", "add group, format: addgroup: groupid"},
    {"groupchat", "group chat, format: groupchat: groupid: message"},
    {"loginout", "login out, format: loginout"}
};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addFriend}
    // {"creategroup", createGroup},
    // {"addgroup", addGroup},
    // {"groupchat", groupChat},
    // {"loginout", loginout}
};



void mainMenu(int clientfd)
{
    cout << "1" << endl;
    help();

    char buffer[1024] = {0};
    for (;;) {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":");
        if (-1 == idx) {
            command = commandbuf;
        } else {
            command = commandbuf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end()) {
            cerr << "invalid command, please input again!" << endl;
            continue;
        }

        // 调用相应的命令处理函数，mainMenu对修改封闭，对扩展开放，添加新功能只需要添加新的命令处理函数即可
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx - 1));
    }
}


// help command handler
void help(int, string)
{
    cout << "=======================command list==========================" << endl;
    for (auto iter = commandMap.begin(); iter != commandMap.end(); ++iter)
    {
        cout << iter->first << " : " << iter->second << endl;
    }
    cout << "============================================================" << endl;
}

// chat command handler
void chat(int clientfd, string commandbuf)
{
    int idx = commandbuf.find(":");
    if (-1 == idx) {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(commandbuf.substr(0, idx).c_str());
    string message = commandbuf.substr(idx + 1, commandbuf.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string request = js.dump();

    cout << "send chat message: " << request << endl;

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "send chat message error!" << endl;
    }
}

// addfriend command handler
void addFriend(int clientfd, string commandbuf)
{
    int friendid = atoi(commandbuf.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string request = js.dump();

    int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
    if (len == -1) {
        cerr << "send addfriend message error!" << endl;
    }
}