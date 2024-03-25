#include <memory>
#include <unordered_map>
class A {
 public:
  A(int a){};
  std::unique_ptr<int> array_;
  //~A(){};
};

int main() {
  std::unordered_map<int, A> a;
  a.emplace(1, A(11));
  return 0;
}