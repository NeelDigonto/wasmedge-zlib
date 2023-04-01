#include <cstdint>
#include <emscripten.h>
#include <iostream>

extern "C" uint32_t fib(uint32_t _n) {
  if (_n <= 1)
    return 1;

  return _n * fib(_n - 1);
}

int main() {
  std::cout << "YOYO" << std::endl;
  return 0;
}