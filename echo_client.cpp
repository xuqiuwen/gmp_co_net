#include <arpa/inet.h>  // for sockaddr_in, inet_pton
#include <unistd.h>     // for read, write, close

#include <cstdlib>  // for atoi
#include <cstring>  // for memset and strlen
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <Port>\n";
    return 1;
  }

  int port = std::atoi(argv[1]);

  // 创建 socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    std::cerr << "Failed to create socket\n";
    return 1;
  }

  // 指定服务器地址和端口
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);  // 使用命令行指定的端口
  if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <=
      0) {  // 服务器的 IP 地址
    std::cerr << "Invalid address/ Address not supported\n";
    return 1;
  }

  // 连接到服务器
  if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    std::cerr << "Connection Failed\n";
    return 1;
  }
  std::cout << "Connected to the server on port " << port << "\n";
  while (true) {
    // 发送数据
    char message[1024];
    scanf("%s", message);
    send(sock, message, strlen(message), 0);
    std::cout << "Message sent\n";

    // 接收服务器的回应
    char buffer[1024] = {0};
    ssize_t bytesReceived = read(sock, buffer, 1024);
    if (bytesReceived > 0) {
      std::cout << "Received from server: " << buffer << std::endl;
    }
  }

  // 关闭 socket
  close(sock);

  return 0;
}
