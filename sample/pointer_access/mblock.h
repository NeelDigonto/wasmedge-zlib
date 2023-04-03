#include <cstdint>
#include <cstdio>

typedef union {
  unsigned char c;
  struct {
    unsigned char b0 : 1;
    unsigned char b1 : 1;
    unsigned char b2 : 1;
    unsigned char b3 : 1;
    unsigned char b4 : 1;
    unsigned char b5 : 1;
    unsigned char b6 : 1;
    unsigned char b7 : 1;
  };
} byte_struct;

struct mblock {
  int32_t i32_sm; // 1
  int32_t i32_md; // 64
  int32_t i32_lg; // 65536

  int32_t ni32_sm;
  int32_t ni32_md;
  int32_t ni32_lg;

  uint32_t u32_sm;
  uint32_t u32_md;
  uint32_t u32_lg;

  uint32_t str_size;
  char *str;

  uint32_t ustr_size;
  unsigned char *ustr;
};

/**
    Printing 52 bytes

    01 00 00 00 40 00 00 00
    00 00 01 00 FF FF FF FF
    C0 FF FF FF 00 00 FF FF
    01 00 00 00 40 00 00 00
    00 00 01 00 18 00 00 00
    A8 0D 01 00 18 00 00 00
    C8 0D 01 00
    Printing 24 bytes

    54 68 69 73 20 69 73 20
    69 6E 73 69 64 65 20 73
    74 72 2E 00 00 00 00 00
    Printing 24 bytes

    54 68 69 73 20 69 73 20
    69 6E 73 69 64 65 20 73
    74 72 2E 00 00 00 00 00
*/

/*
    Printing 52 bytes

    01 00 00 00 40 00 00 00
    00 00 01 00 FF FF FF FF
    C0 FF FF FF 00 00 FF FF
    01 00 00 00 40 00 00 00
    00 00 01 00 18 00 00 00
    D8 0A 01 00 18 00 00 00
    F8 0A 01 00
*/

void print_bytes(void *ptr, int size) {
  printf("Printing %d bytes\n", size);

  unsigned char *p = reinterpret_cast<unsigned char *>(ptr);
  int i;
  for (i = 0; i < size; i++) {

    if (i % 8 == 0)
      printf("\n");

    printf("%02hhX ", p[i]);
  }
  printf("\n");
}