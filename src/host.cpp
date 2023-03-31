#include <cassert>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <wasmedge/wasmedge.h>
#include <zlib.h>

constexpr bool isLittleEndian() {
  short int number = 0x1;
  char *numPtr = (char *)&number;
  return (numPtr[0] == 1);
}

struct module_z_stream {
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

  uint32_t data_type;

  uint32_t adler;
  uint32_t reserved;
}; // 56

z_stream GetHostZStream(module_z_stream *_module_z_stream) {}

module_z_stream GetModuleZStream(z_stream *_z_stream) {}

WasmEdge_Result
WasmEdge_deflateInit(void *Data,
                     const WasmEdge_CallingFrameContext *CallFrameCxt,
                     const WasmEdge_Value *In, WasmEdge_Value *Out) {
  int32_t Val1 = WasmEdge_ValueGetI32(In[0]);
  int32_t Val2 = WasmEdge_ValueGetI32(In[1]);
  printf("Host function \"Add\": %d + %d\n", Val1, Val2);
  Out[0] = WasmEdge_ValueGenI32(Val1 + Val2);
  return WasmEdge_Result_Success;
}

void Register_deflateInit(WasmEdge_ModuleInstanceContext *_module_context) {

  WasmEdge_String HostFuncName = WasmEdge_StringCreateByCString("deflateInit");

  enum WasmEdge_ValType ParamList[2] = {WasmEdge_ValType_I32,
                                        WasmEdge_ValType_I32};
  enum WasmEdge_ValType ReturnList[1] = {WasmEdge_ValType_I32};

  WasmEdge_FunctionTypeContext *HostFType =
      WasmEdge_FunctionTypeCreate(ParamList, 2, ReturnList, 1);
  WasmEdge_FunctionInstanceContext *HostFunc =
      WasmEdge_FunctionInstanceCreate(HostFType, WasmEdge_deflateInit, NULL, 0);

  WasmEdge_FunctionTypeDelete(HostFType);

  WasmEdge_ModuleInstanceAddFunction(_module_context, HostFuncName, HostFunc);
  WasmEdge_StringDelete(HostFuncName);
}

int main() {
  if (!isLittleEndian())
    throw std::runtime_error("Will support Big Endian Later.");

  std::unordered_map<uint32_t, z_stream *> stream_map;

  WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
  WasmEdge_ConfigureAddHostRegistration(ConfCxt,
                                        WasmEdge_HostRegistration_Wasi);

  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, NULL);

  WasmEdge_String ExportName = WasmEdge_StringCreateByCString("env");
  WasmEdge_ModuleInstanceContext *HostModCxt =
      WasmEdge_ModuleInstanceCreate(ExportName);

  Register_deflateInit(HostModCxt);
  // Register_deflate(HostModCxt);
  // Register_deflateEnd(HostModCxt);
  //
  // Register_inflateInit(HostModCxt);
  // Register_inflate(HostModCxt);
  // Register_inflateEnd(HostModCxt);

  WasmEdge_VMRegisterModuleFromImport(VMCxt, HostModCxt);

  WasmEdge_String EntryPoint = WasmEdge_StringCreateByCString("test");
  WasmEdge_Value EntryPointParams[0], EntryPointReturns[1];

  WasmEdge_Result Res =
      WasmEdge_VMRunWasmFromFile(VMCxt, "./module.wasm", EntryPoint,
                                 EntryPointParams, 0, EntryPointReturns, 1);

  if (WasmEdge_ResultOK(Res)) {
    printf("Get result: %d\n", WasmEdge_ValueGetI32(EntryPointReturns[0]));
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_ConfigureDelete(ConfCxt);
  WasmEdge_StringDelete(EntryPoint);
  return 0;
}