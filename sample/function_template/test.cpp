#include <iostream>

void deflate(int _param) { printf("Deflate Param: %d\n", _param); }
void inflate(int _param) { printf("Inflate Param: %d\n", _param); }

template <auto &Func> void wrapper(int _param) { Func(_param); }

typedef typeof wrapper<deflate> Deflate;

int main() {
  int var = 69;
  wrapper<deflate>(var);
  wrapper<inflate>(var);

  var = 420;

  Deflate(var);
}