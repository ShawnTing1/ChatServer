#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;
using namespace muduo::net;
using json = nlohmann::json;
#include <functional>
#include <muduo/base/Logging.h>

#include <iostream>

// 处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int sig)
{
    ChatService::instance()->reset();
    exit(0);
}
int main()
{
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6666);
    Socket socket;
    socket.setReuseAddr(true); // 设置 SO_REUSEADDR 选项
    TcpServer server(&amp;loop, listenAddr, "ChatServer", socket);

    ChatServer chatServer(&amp;loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}
