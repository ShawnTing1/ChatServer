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
// void readTaskHandler(int clientfd);
// 获取系统时间
// string getCurrentTime();
// 主聊天页面程序
// void mainMenu();

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
                            // 记录当前登陆用户的id，name，state
                            g_currentUser.setId(id);  
                            g_currentUser.setName(response_js["name"]);
                            // 记录当前登陆用户的好友列表信息
                            if (response_js.contains("friends")) {
                                vector<User> vec;
                                for (json item : response_js["friends"]) {
                                    User user(item["id"], item["name"], item["state"]);
                                    vec.push_back(user);
                                }
                                g_currentUserFriendList = vec;
                            }

                            // 记录当前登陆用户的群组列表信息
                            if (response_js.contains("groups")) {
                                vector<string> vec1 = response_js["groups"];
                                for (string& groupstr : vec1) {
                                    json grpjs = json::parse(groupstr);
                                    Group group;
                                    group.setId(grpjs["id"]);
                                    group.setName(grpjs["name"]);
                                    group.setDesc(grpjs["desc"]);

                                    vector<string> vec2 = grpjs["users"];
                                    for (string& userstr : vec2) {
                                        json usrjs = json::parse(userstr);
                                        GroupUser user;
                                        user.setId(usrjs["id"]);
                                        user.setName(usrjs["name"]);
                                        user.setState(usrjs["state"]);
                                        group.getUsers().push_back(user);
                                    }
                                    g_currentUserGroupList.push_back(group);
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
                                // thread readTask(readTaskHandler, clientfd);
                                // readTask.detach();

                                // 进入主聊天页面
                                // mainMenu();
                            }
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