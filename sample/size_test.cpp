#if defined(__EMSCRIPTEN__)
#include "../include/zlib.h"
#include <emscripten.h>
#else
#include <zlib.h>
#endif

#include <iostream>

#define PRINT_SIZE(x) std::cout << #x << " : " << sizeof(x) << std::endl;

int main() {
  PRINT_SIZE(z_const unsigned char *);
  PRINT_SIZE(uint32_t);
  PRINT_SIZE(unsigned long);
  PRINT_SIZE(unsigned char *);
  PRINT_SIZE(z_const char *);
  PRINT_SIZE(struct internal_state *);
  PRINT_SIZE(alloc_func);
  PRINT_SIZE(free_func);
  PRINT_SIZE(void *);
  PRINT_SIZE(int);
  PRINT_SIZE(z_stream);
}
// em++ sample/size_test.cpp -o size_test.wasm -sSTANDALONE_WASM && wasmedge
// size_test.wasm

/**
** fathomless@vividecstasy:~/repo/wasmedge-zlib$ em++ sample/size_test.cpp -o
*size_test.wasm -sSTANDALONE_WASM && wasmedge size_test.wasm
** z_const unsigned char * : 4
** uint32_t : 4
** unsigned long : 4
** unsigned char * : 4
** z_const char * : 4
** struct internal_state * : 4
** alloc_func : 4
** free_func : 4
** void * : 4
** int : 4
** fathomless@vividecstasy:~/repo/wasmedge-zlib$ g++ sample/size_test.cpp -o
*size_test && ./size_test
** z_const unsigned char * : 8
** uint32_t : 4
** unsigned long : 8
** unsigned char * : 8
** z_const char * : 8
** struct internal_state * : 8
** alloc_func : 8
** free_func : 8
** void * : 8
** int : 4
** fathomless@vividecstasy:~/repo/wasmedge-zlib$
*/