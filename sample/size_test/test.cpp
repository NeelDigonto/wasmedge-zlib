#include <cstdint>
#include <iostream>

struct wasm_z_stream {
  uint32_t next_in;
  uint32_t avail_in;
  uint32_t total_in;

  uint32_t next_out;
  uint32_t avail_out;
  uint32_t total_out;

  uint32_t msg;
  uint32_t state;

  uint32_t zalloc;
  uint32_t zfree;
  uint32_t opaque;

  int32_t data_type;

  uint32_t adler;
  uint32_t reserved;
}; // 56

int main() { std::cout << sizeof(wasm_z_stream) << std::endl; }