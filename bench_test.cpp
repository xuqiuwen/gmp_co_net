#include <arpa/inet.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

std::atomic_int successfulRequests(0);

int duration = 10;  // 测试持续时间，单位秒

void testConnection() {
  std::string address = "127.0.0.1";  // 服务器地址
  int port = 8888;                    // 端口号

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    std::cerr << "Failed to create socket\n";
    return;
  }

  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
    std::cerr << "Invalid address/ Address not supported\n";
    close(sock);
    return;
  }

  if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    std::cerr << "Connection Failed\n";
    close(sock);
    return;
  }

  auto startTime = std::chrono::high_resolution_clock::now();
  auto endTime = startTime + std::chrono::seconds(duration);

  while (std::chrono::high_resolution_clock::now() < endTime) {
    const char* message = "helloworld";
    send(sock, message, strlen(message), 0);

    char buffer[1024] = {0};
    ssize_t bytesReceived = read(sock, buffer, sizeof(buffer));
    if (bytesReceived > 0) {
      successfulRequests++;
    }
  }

  close(sock);
}

int main() {
  int concurrentConnections = 4;  // 并发连接数，cpu 才6个核

  std::vector<std::thread> threads;

  for (int i = 0; i < concurrentConnections; ++i) {
    threads.emplace_back(testConnection);
  }

  for (auto& t : threads) {
    t.join();
  }

  std::cout << "Test finished.\n";
  std::cout << "Successful requests: " << successfulRequests << "\n";
  std::cout << "QPS: " << static_cast<double>(successfulRequests) / duration
            << std::endl;

  return 0;
}
