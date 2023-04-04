#if defined(__EMSCRIPTEN__)
#include "../include/zlib.h"
#include <emscripten.h>
#else
#include <iostream>
#include <stdexcept>
#include <zlib.h>
#endif
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <random>
#include <type_traits>
#include <vector>

// static_assert(sizeof(unsigned char) == sizeof(std::byte), "Unsgined char and
// std::byte are not equal in size");

#ifdef __EMSCRIPTEN__
#define PRESERVE EMSCRIPTEN_KEEPALIVE
#else
#define PRESERVE
#endif

static constexpr size_t DATA_SIZE = 1 * 1024 * 1024;
static constexpr size_t BUFFER_SIZE = 16'384; // 16 * 1024

constexpr auto randChar = []() -> char {
  constexpr char charset[] = "0123456789"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "abcdefghijklmnopqrstuvwxyz";
  constexpr size_t max_index = (sizeof(charset) - 1);
  return charset[rand() % max_index];
};

void *custom_malloc(voidpf opaque, uInt items, uInt size) {

  auto add = malloc(items * size);
#ifndef __EMSCRIPTEN__
  std::cout << "zalloc : " << add << " = " << items * size << std::endl;
#endif
  return add;
}

void custom_free(voidpf opaque, voidpf address) {
#ifndef __EMSCRIPTEN__
  std::cout << "zfree : " << address << std::endl;
#endif
  return free(address);
}

int InitDeflateZStream(z_stream &strm, int level) {
  strm.zalloc = custom_malloc;
  strm.zfree = custom_free;
  strm.opaque = Z_NULL;

  int ret = deflateInit(&strm, level);

#ifndef __EMSCRIPTEN__
  if (ret != Z_OK)
    throw std::runtime_error("'deflateInit' failed!");
#endif

  return ret;
}

int InitInflateZStream(z_stream &strm) {
  strm.zalloc = custom_malloc;
  strm.zfree = custom_free;
  strm.opaque = Z_NULL;

  int ret = inflateInit(&strm);

#ifndef __EMSCRIPTEN__
  if (ret != Z_OK)
    throw std::runtime_error("'inflateInit' failed!");
#endif

  return ret;
}

template <typename T>
std::vector<unsigned char> Deflate(const std::vector<T> &source,
                                   int level = -1) {

  int ret, flush;
  z_stream strm;
  ret = InitDeflateZStream(strm, level);
  const std::size_t src_size = source.size() * sizeof(T);
  std::size_t out_buffer_size = src_size / 3 + 16;
  std::vector<unsigned char> out_buffer(out_buffer_size, {});

  strm.avail_in = src_size;
  strm.next_in = reinterpret_cast<unsigned char *>(
      const_cast<std::remove_const_t<T> *>(source.data()));
  strm.avail_out = out_buffer.size();
  strm.next_out = out_buffer.data();

  do {

    if (strm.avail_out == 0) {
      const std::size_t extension_size = src_size / 3 + 16;
      strm.avail_out = extension_size;
      out_buffer.resize(out_buffer_size + extension_size, {});
      strm.next_out = std::next(out_buffer.data(), out_buffer_size);
      out_buffer_size += extension_size;
    }

    ret = deflate(&strm, Z_FINISH);

#ifndef __EMSCRIPTEN__
    if (ret == Z_STREAM_ERROR)
      throw std::runtime_error("Zlib Stream Error!");
#endif
  } while (ret != Z_STREAM_END);

  deflateEnd(&strm);
  out_buffer.resize(out_buffer_size - strm.avail_out);

  return out_buffer;
}

template <typename T>
std::vector<T> Inflate(const std::vector<unsigned char> &source) {

  int ret, flush;
  z_stream strm;
  ret = InitInflateZStream(strm);
  const std::size_t src_size = source.size();
  std::size_t out_buffer_size = src_size / 3 + 16;
  std::vector<unsigned char> out_buffer(out_buffer_size, {});

  strm.avail_in = src_size;
  strm.next_in = const_cast<unsigned char *>(source.data());
  strm.avail_out = out_buffer.size();
  strm.next_out = out_buffer.data();

  do {

    if (strm.avail_out == 0) {
      const std::size_t extension_size = src_size / 3 + 16;
      strm.avail_out = extension_size;
      out_buffer.resize(out_buffer_size + extension_size, {});
      strm.next_out = std::next(out_buffer.data(), out_buffer_size);
      out_buffer_size += extension_size;
    }

    ret = inflate(&strm, Z_FINISH);

#ifndef __EMSCRIPTEN__
    if (ret == Z_STREAM_ERROR)
      throw std::runtime_error("Zlib Stream Error!");
#endif
  } while (ret != Z_STREAM_END);

  inflateEnd(&strm);
  out_buffer_size -= strm.avail_out;

  std::vector<T> ret_buffer(reinterpret_cast<T *>(out_buffer.data()),
                            std::next(reinterpret_cast<T *>(out_buffer.data()),
                                      (out_buffer_size / sizeof(T))));

  return ret_buffer;
}

extern "C" int PRESERVE test() {
  std::vector<char> data(DATA_SIZE, {});
  std::generate_n(std::begin(data), DATA_SIZE, randChar);

#ifndef __EMSCRIPTEN__
  std::cout << "Compressing Buffer of size : " << DATA_SIZE << "B" << std::endl;
#endif
  const auto compressed_buffer = Deflate(data, 6);

#ifndef __EMSCRIPTEN__
  std::cout << "Decompressing Buffer of size : " << compressed_buffer.size()
            << "B" << std::endl;
#endif
  const auto decompressed_buffer = Inflate<char>(compressed_buffer);

  auto comp_res = data == decompressed_buffer;
#ifndef __EMSCRIPTEN__
  std::cout << (comp_res ? "Success" : "Fail") << std::endl;
#endif

  return comp_res;
}

int main() {
  test();
  return 0;
}

// g++ src/module.cpp -lz -o module && ./module
// em++ module.cpp -O2 -o module.wasm -sSTANDALONE_WASM
// -sWARN_ON_UNDEFINED_SYMBOLS=0

/*
        fathomless@vividecstasy:~/repo/wasmedge-zlib/src$ g++ -O2 module.cpp -o
   module -lz && ./module Compressing Buffer of size : 1048576B zalloc :
   0x563ffacb22c0 = 5952 zalloc : 0x563ffacb3a10 = 65536 zalloc : 0x563ffacc3a20
   = 65536 zalloc : 0x563ffacd3a30 = 65536 zalloc : 0x563fface3a40 = 65536 zfree
   : 0x563fface3a40 zfree : 0x563ffacd3a30 zfree : 0x563ffacc3a20 zfree :
   0x563ffacb3a10 zfree : 0x563ffacb22c0 Decompressing Buffer of size : 788616B
        zalloc : 0x563ffacb22c0 = 7160
        zalloc : 0x563ffacf41b0 = 32768
        zfree : 0x563ffacf41b0
        zfree : 0x563ffacb22c0
        Success */

/**
     extern "C" int PRESERVE test() {
std::vector<char> data(DATA_SIZE, {});
std::generate_n(std::begin(data), DATA_SIZE, randChar);

std::cout << "Compressing Buffer of size : " << DATA_SIZE << "B" << std::endl;
const auto compressed_buffer = Deflate(data, 6);

std::cout << "Decompressing Buffer of size : " << compressed_buffer.size()
        << "B" << std::endl;

const auto decompressed_buffer = Inflate<char>(compressed_buffer);

auto comp_res =
  (data == decompressed_buffer) && (compressed_buffer.size() < data.size());
std::cout << (comp_res ? "Success" : "Fail") << std::endl;
return comp_res;
}
*/