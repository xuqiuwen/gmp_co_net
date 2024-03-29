#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "./LibGoRoutine.h"

#define BUFFER_SIZE 50

Task HandleClient(int clientFd) {
  char buffer[BUFFER_SIZE];
  // std::cerr << "开始处理客户端连接" << clientFd << std::endl;

  while (true) {
    memset(buffer, 0, BUFFER_SIZE);
    // 读取客户端发送的数据
    ssize_t bytesRead = GO_READ(clientFd, buffer, BUFFER_SIZE);
    // std::cerr << "读取字节数" << bytesRead << std::endl;
    if (bytesRead <= 0) {
      // 读取失败或客户端断开连接
      break;
    }

    // 将接收到的数据回发给客户端
    ssize_t bytesWrite = GO_WRITE(clientFd, buffer, bytesRead);
    // std::cerr << "发送字节数" << bytesWrite << std::endl;
    // std::cerr << "一轮读写完成\n";
  }
  // std::cerr << "关闭客户端连接" << clientFd << std::endl;
  //  关闭客户端连接
  close(clientFd);
}

int main(int argc, char* argv[]) {
  // 检查命令行参数
  if (argc < 2) {
    // std::cerr << "Usage: " << argv[0] << " <Port>\n";
    return 1;
  }

  int port = std::atoi(argv[1]);

  // 启动 Runtime 环境
  GO_START;

  int serverFd, clientFd;
  struct sockaddr_in serverAddr, clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);

  // 创建 socket
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    // std::cerr << "Cannot open socket\n";
    return 1;
  }

  // 绑定地址和端口
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons(port);  // 选择一个端口

  if (bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    // std::cerr << "Bind failed\n";
    close(serverFd);
    return 1;
  }

  // 开始监听
  if (listen(serverFd, 5) < 0) {
    // std::cerr << "Listen failed\n";
    close(serverFd);
    return 1;
  }

  std::cerr << "Server listening on port " << port << "\n";

  while (true) {
    // 接受客户端连接
    clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd < 0) {
      // std::cerr << "Accept failed\n";
      continue;
    }

    std::cerr << "Client connected\n";

    // 为每个客户端创建一个协程进行处理
    GO(HandleClient(clientFd));
  }
  // std::cerr << "关闭服务器" << clientFd << std::endl;
  //  清理
  close(serverFd);
  return 0;
}