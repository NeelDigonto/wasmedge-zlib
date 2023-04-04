#include <cassert>
#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <wasmedge/wasmedge.h>
#include <zlib.h>

inline bool isLittleEndian() {
  short int number = 0x1;
  char *numPtr = (char *)&number;
  return (numPtr[0] == 1);
}

struct Util {
  std::unordered_map<uint32_t, z_stream *> stream_map;
};

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

/* wasm_z_stream *GetWasmZStream(const WasmEdge_CallingFrameContext
 *CallFrameCxt, uint32_t _wasm_z_stream_ptr, ) {} */

void ValidateWasmZStream(const WasmEdge_CallingFrameContext *CallFrameCxt,
                         uint32_t _wasm_z_stream_ptr,
                         uint32_t _wasm_version_ptr,
                         int32_t _wasm_stream_size) {
  WasmEdge_MemoryInstanceContext *MemCxt =
      WasmEdge_CallingFrameGetMemoryInstance(CallFrameCxt, 0);
  wasm_z_stream *wasm_stream =
      reinterpret_cast<wasm_z_stream *>(WasmEdge_MemoryInstanceGetPointer(
          MemCxt, _wasm_z_stream_ptr, sizeof(wasm_z_stream)));
  const char *wasm_ZLIB_VERSION = reinterpret_cast<const char *>(
      WasmEdge_MemoryInstanceGetPointer(MemCxt, _wasm_version_ptr, 1));

  // Check major version of zlib and assert sizeof z_stream == 56

  if (wasm_ZLIB_VERSION[0] != ZLIB_VERSION[0])
    throw std::runtime_error(std::string("Host(") + wasm_ZLIB_VERSION[0] +
                             ") and Wasm Modue(" + ZLIB_VERSION[0] +
                             ") zlib Major Version does not match!");

  if (_wasm_stream_size != 56)
    throw std::runtime_error(std::string("WASM sizeof(z_stream) != 56 but ") +
                             std::to_string(_wasm_stream_size));
}

z_stream *GetInitHostZStream(const WasmEdge_CallingFrameContext *CallFrameCxt,
                             uint32_t _wasm_z_stream_ptr) {
  WasmEdge_MemoryInstanceContext *MemCxt =
      WasmEdge_CallingFrameGetMemoryInstance(CallFrameCxt, 0);
  wasm_z_stream *wasm_stream =
      reinterpret_cast<wasm_z_stream *>(WasmEdge_MemoryInstanceGetPointer(
          MemCxt, _wasm_z_stream_ptr, sizeof(wasm_z_stream)));

  z_stream *stream = new z_stream;
  // ignore wasm custom zmalloc and zfree
  stream->zalloc = Z_NULL;
  stream->zfree = Z_NULL;
  // ignore opaque since zmalloc and zfree was ignored
  stream->opaque = Z_NULL;

  return stream;
}

WasmEdge_Result
WasmEdge_deflateInit_(void *Data,
                      const WasmEdge_CallingFrameContext *CallFrameCxt,
                      const WasmEdge_Value *In, WasmEdge_Value *Out) {
  uint32_t wasm_z_stream_ptr = (uint32_t)WasmEdge_ValueGetI32(In[0]);
  int32_t wasm_level = WasmEdge_ValueGetI32(In[1]);
  uint32_t wasm_version_ptr = (uint32_t)WasmEdge_ValueGetI32(In[2]);
  int32_t wasm_stream_size = WasmEdge_ValueGetI32(In[3]);

  ValidateWasmZStream(CallFrameCxt, wasm_z_stream_ptr, wasm_version_ptr,
                      wasm_stream_size);
  auto stream = GetInitHostZStream(CallFrameCxt, wasm_z_stream_ptr);

  const auto z_res =
      deflateInit_(stream, wasm_level, ZLIB_VERSION, sizeof(z_stream));

  Out[0] = WasmEdge_ValueGenI32(z_res);

  reinterpret_cast<Util *>(Data)->stream_map.insert(
      {wasm_z_stream_ptr, stream});

  return WasmEdge_Result_Success;
}

WasmEdge_Result
WasmEdge_inflateInit_(void *Data,
                      const WasmEdge_CallingFrameContext *CallFrameCxt,
                      const WasmEdge_Value *In, WasmEdge_Value *Out) {
  uint32_t wasm_z_stream_ptr = (uint32_t)WasmEdge_ValueGetI32(In[0]);
  uint32_t wasm_version_ptr = (uint32_t)WasmEdge_ValueGetI32(In[1]);
  int32_t wasm_stream_size = WasmEdge_ValueGetI32(In[2]);

  ValidateWasmZStream(CallFrameCxt, wasm_z_stream_ptr, wasm_version_ptr,
                      wasm_stream_size);
  auto stream = GetInitHostZStream(CallFrameCxt, wasm_z_stream_ptr);

  const auto z_res = inflateInit_(stream, ZLIB_VERSION, sizeof(z_stream));

  Out[0] = WasmEdge_ValueGenI32(z_res);

  reinterpret_cast<Util *>(Data)->stream_map.insert(
      {wasm_z_stream_ptr, stream});

  return WasmEdge_Result_Success;
}

template <auto &Func>
WasmEdge_Result WasmEdge_algo1(void *Data,
                               const WasmEdge_CallingFrameContext *CallFrameCxt,
                               const WasmEdge_Value *In, WasmEdge_Value *Out) {
  uint32_t wasm_z_stream_ptr = (uint32_t)WasmEdge_ValueGetI32(In[0]);
  int32_t wasm_flush = WasmEdge_ValueGetI32(In[1]);

  auto stream_map_it =
      reinterpret_cast<Util *>(Data)->stream_map.find(wasm_z_stream_ptr);

  if (stream_map_it == reinterpret_cast<Util *>(Out)->stream_map.end())
    throw std::runtime_error("ZStream not found in map");

  auto stream = stream_map_it->second;

  WasmEdge_MemoryInstanceContext *MemCxt =
      WasmEdge_CallingFrameGetMemoryInstance(CallFrameCxt, 0);
  uint8_t *wasm_mem =
      WasmEdge_MemoryInstanceGetPointer(MemCxt, 0, 128 * 1024 * 1024);

  wasm_z_stream *wasm_stream =
      reinterpret_cast<wasm_z_stream *>(wasm_mem + wasm_z_stream_ptr);

  stream->avail_in = wasm_stream->avail_in;
  stream->avail_out = wasm_stream->avail_out;
  stream->next_in = wasm_mem + wasm_stream->next_in;
  stream->next_out = wasm_mem + wasm_stream->next_out;

  const auto z_res = Func(stream, wasm_flush);

  // now write it to wasm memory
  wasm_stream->avail_in = stream->avail_in;
  wasm_stream->avail_out = stream->avail_out;
  wasm_stream->next_in = stream->next_in - wasm_mem;
  wasm_stream->next_out = stream->next_out - wasm_mem;

  Out[0] = WasmEdge_ValueGenI32(z_res);

  return WasmEdge_Result_Success;
}

template <auto &Func>
WasmEdge_Result
WasmEdge_ZlibEnd(void *Data, const WasmEdge_CallingFrameContext *CallFrameCxt,
                 const WasmEdge_Value *In, WasmEdge_Value *Out) {
  uint32_t wasm_z_stream_ptr = (uint32_t)WasmEdge_ValueGetI32(In[0]);

  WasmEdge_MemoryInstanceContext *MemCxt =
      WasmEdge_CallingFrameGetMemoryInstance(CallFrameCxt, 0);
  wasm_z_stream *wasm_stream =
      reinterpret_cast<wasm_z_stream *>(WasmEdge_MemoryInstanceGetPointer(
          MemCxt, wasm_z_stream_ptr, sizeof(wasm_z_stream)));

  auto stream_map_it =
      reinterpret_cast<Util *>(Data)->stream_map.find(wasm_z_stream_ptr);

  if (stream_map_it == reinterpret_cast<Util *>(Data)->stream_map.end())
    throw std::runtime_error("ZStream not found in map");

  const auto z_res = Func(stream_map_it->second);
  Out[0] = WasmEdge_ValueGenI32(z_res);

  reinterpret_cast<Util *>(Data)->stream_map.erase(stream_map_it);

  return WasmEdge_Result_Success;
}

static void
RegisterHostFunction(const std::string &_function_name,
                     WasmEdge_HostFunc_t _func_pointer,
                     std::vector<WasmEdge_ValType> _params_list,
                     std::vector<WasmEdge_ValType> _return_list, Util *_util,
                     WasmEdge_ModuleInstanceContext *_module_context) {
  WasmEdge_String HostFuncName =
      WasmEdge_StringCreateByCString(_function_name.c_str());

  WasmEdge_FunctionTypeContext *HostFType =
      WasmEdge_FunctionTypeCreate(_params_list.data(), _params_list.size(),
                                  _return_list.data(), _return_list.size());
  WasmEdge_FunctionInstanceContext *HostFunc =
      WasmEdge_FunctionInstanceCreate(HostFType, _func_pointer, _util, 0);
  WasmEdge_ModuleInstanceAddFunction(_module_context, HostFuncName, HostFunc);
  WasmEdge_FunctionTypeDelete(HostFType);
  WasmEdge_StringDelete(HostFuncName);
}

int main() {
  if (!isLittleEndian())
    throw std::runtime_error("Will support Big Endian Later.");

  Util util;

  WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
  WasmEdge_ConfigureAddHostRegistration(ConfCxt,
                                        WasmEdge_HostRegistration_Wasi);
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, NULL);

  WasmEdge_String ExportName = WasmEdge_StringCreateByCString("env");
  WasmEdge_ModuleInstanceContext *HostModCxt =
      WasmEdge_ModuleInstanceCreate(ExportName);

  WasmEdge_Limit MemLimit = {
      .HasMax = true, .Shared = false, .Min = 16 * 128, .Max = 16 * 128};
  WasmEdge_MemoryTypeContext *HostMType = WasmEdge_MemoryTypeCreate(MemLimit);
  WasmEdge_MemoryInstanceContext *HostMemory =
      WasmEdge_MemoryInstanceCreate(HostMType);
  WasmEdge_String MemoryName = WasmEdge_StringCreateByCString("memory");
  WasmEdge_ModuleInstanceAddMemory(HostModCxt, MemoryName, HostMemory);
  WasmEdge_MemoryTypeDelete(HostMType);

  /**
   * @brief
   * Z_EXTERN int Z_EXPORT deflateInit_(z_stream *strm, int level,
                                   const char *version, int stream_size);
   */
  RegisterHostFunction("deflateInit_", WasmEdge_deflateInit_,
                       {WasmEdge_ValType_I32, WasmEdge_ValType_I32,
                        WasmEdge_ValType_I32, WasmEdge_ValType_I32},
                       {WasmEdge_ValType_I32}, &util, HostModCxt);
  /**
   * @brief
   * ZEXTERN int ZEXPORT deflate OF((z_streamp strm, int flush));
   */
  RegisterHostFunction("deflate", WasmEdge_algo1<deflate>,
                       {WasmEdge_ValType_I32, WasmEdge_ValType_I32},
                       {WasmEdge_ValType_I32}, &util, HostModCxt);

  /**
   * @brief
   * ZEXTERN int ZEXPORT deflateEnd OF((z_streamp strm));
   */
  RegisterHostFunction("deflateEnd", WasmEdge_ZlibEnd<deflateEnd>,
                       {WasmEdge_ValType_I32}, {WasmEdge_ValType_I32}, &util,
                       HostModCxt);

  /**
   * @brief
   * ZEXTERN int ZEXPORT inflateInit_ OF((z_streamp strm,
                                     const char *version, int stream_size));
   */
  RegisterHostFunction("inflateInit_", WasmEdge_inflateInit_,
                       {
                           WasmEdge_ValType_I32,
                           WasmEdge_ValType_I32,
                           WasmEdge_ValType_I32,
                       },
                       {WasmEdge_ValType_I32}, &util, HostModCxt);

  /**
   * @brief
   * ZEXTERN int ZEXPORT inflate OF((z_streamp strm, int flush));
   */

  RegisterHostFunction("inflate", WasmEdge_algo1<inflate>,
                       {
                           WasmEdge_ValType_I32,
                           WasmEdge_ValType_I32,
                       },
                       {WasmEdge_ValType_I32}, &util, HostModCxt);

  /**
   * @brief
   * ZEXTERN int ZEXPORT inflateEnd OF((z_streamp strm));
   */
  RegisterHostFunction("inflateEnd", WasmEdge_ZlibEnd<inflateEnd>,
                       {WasmEdge_ValType_I32}, {WasmEdge_ValType_I32}, &util,
                       HostModCxt);

  WasmEdge_VMRegisterModuleFromImport(VMCxt, HostModCxt);

  WasmEdge_String EntryPoint = WasmEdge_StringCreateByCString("test");
  WasmEdge_Value EntryPointParams[0], EntryPointReturns[1];
  WasmEdge_Result Res =
      WasmEdge_VMRunWasmFromFile(VMCxt, "./module.wasm", EntryPoint,
                                 EntryPointParams, 0, EntryPointReturns, 1);
  if (WasmEdge_ResultOK(Res)) {
    const auto test_res = WasmEdge_ValueGetI32(EntryPointReturns[0]);
    printf("Test Result : %s\n", test_res ? "Success" : "Failed");
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_ConfigureDelete(ConfCxt);
  WasmEdge_StringDelete(EntryPoint);
  return 0;
}