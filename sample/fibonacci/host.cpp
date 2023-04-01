#include <cassert>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <unordered_map>
#include <wasmedge/wasmedge.h>
#include <zlib.h>

int main() {

  WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
  WasmEdge_ConfigureAddHostRegistration(ConfCxt,
                                        WasmEdge_HostRegistration_Wasi);

  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, NULL);

  WasmEdge_String ExportName = WasmEdge_StringCreateByCString("env");
  WasmEdge_ModuleInstanceContext *HostModCxt =
      WasmEdge_ModuleInstanceCreate(ExportName);

  WasmEdge_VMRegisterModuleFromImport(VMCxt, HostModCxt);

  WasmEdge_String EntryPoint = WasmEdge_StringCreateByCString("_start");
  WasmEdge_Value EntryPointParams[0], EntryPointReturns[1];

  WasmEdge_Result Res =
      WasmEdge_VMRunWasmFromFile(VMCxt, "./fib.wasm", EntryPoint,
                                 EntryPointParams, 0, EntryPointReturns, 0);

  if (WasmEdge_ResultOK(Res)) {
    // printf("Get result: %d\n", WasmEdge_ValueGetI32(EntryPointReturns[0]));
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_ConfigureDelete(ConfCxt);
  WasmEdge_StringDelete(EntryPoint);
  return 0;
}