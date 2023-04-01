#include <cstdint>
#include <emscripten.h>
#include <iostream>
// #include <stdio.h>

struct vec2i {
  int32_t a;
  int32_t b;
  int32_t c;
};

//  int alterVec(vec2i *veca, int offset);

// extern "C" void print(vec2i *avec);
//  extern "C" int alter(vec2i *avec, int);

extern "C" uint32_t EMSCRIPTEN_KEEPALIVE test() {
  // printf("Yoo");
  vec2i veca{10, 20, 30};
  char str[50] = "yoyo";
  printf("%s", str);
  // alter(&veca, 5);
  // print(&veca);
  return 1;
}

int main() {
  test();
  // std::cout << "Yoo" << std::endl;
}

/* int main() {


  printf("Before call a=%d, b=%d, c=%d", veca.a, veca.b, veca.c);

  // auto out = alterVec(&veca, 5);

  printf("After call a=%d, b=%d, c=%d", veca.a, veca.b, veca.c);

  test();

  return 1;
}
 */

// em++ module.cpp -O2 -o module.wasm -sSTANDALONE_WASM -sSIDE_MODULE