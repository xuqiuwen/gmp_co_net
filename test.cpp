#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <unordered_map>
int main() {
  std::jthread a;
  std::cout << a.get_id() << std::endl;
  a = std::jthread([]() { return 0; });
  std::cout << a.get_id() << std::endl;
  return 0;
}