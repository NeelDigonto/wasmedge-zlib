#include <iostream>
#include <unordered_map>
using namespace std;

struct Util {
  std::unordered_map<int, char *> stream_map;
};

void func(Util *util) {
  auto chp = new char;
  printf("%p\n", chp);
  util->stream_map.insert({1, chp});
  cout << "Yoo" << endl;

  auto it = util->stream_map.find(1);
  printf("%p\n", it->second);
}

int main() {
  Util util;
  func(&util);
}