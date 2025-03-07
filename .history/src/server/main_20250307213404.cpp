#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

// 处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int sig)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char** argv)
{
    if (argc > 3) {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6666" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的服务器端的ip和端口
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}
