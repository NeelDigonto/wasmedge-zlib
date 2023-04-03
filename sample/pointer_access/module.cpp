#include "./mblock.h"
#include <cstdint>
#include <cstring>
#include <emscripten.h>
#include <iostream>
// #include <stdio.h>

extern "C" void hostFunc(mblock *var);

extern "C" uint32_t EMSCRIPTEN_KEEPALIVE test() {
  mblock obj;

  obj.i32_sm = 1;
  obj.i32_md = 64;
  obj.i32_lg = 65536;

  obj.ni32_sm = -1;
  obj.ni32_md = -64;
  obj.ni32_lg = -65536;

  obj.u32_sm = 1;
  obj.u32_md = 64;
  obj.u32_lg = 65536;

  obj.str_size = 24;
  obj.str = new char[obj.str_size];
  strncpy(obj.str, "This is inside str.", obj.str_size);

  obj.ustr_size = 24;
  obj.ustr = new unsigned char[obj.ustr_size];
  strncpy(reinterpret_cast<char *>(obj.ustr), "This is inside str.",
          obj.str_size);

  hostFunc(&obj);

  // printf("%d", &obj);

  // print_bytes(&obj, sizeof(mblock));
  // print_bytes(obj.str, obj.str_size);
  // print_bytes(obj.ustr, obj.ustr_size);

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

// em++ module.cpp -O2 -o module.wasm -sSTANDALONE_WASM
// -sWARN_ON_UNDEFINED_SYMBOLS=0 -sIMPORTED_MEMORY -sINITIAL_MEMORY=128MB
// -sALLOW_MEMORY_GROWTH=0 && g++ -O2 host.cpp -o host
// -lwasmedge && ./host