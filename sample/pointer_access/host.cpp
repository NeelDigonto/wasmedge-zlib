
#include <iostream>
#include <stdio.h>
#include <wasmedge/wasmedge.h>

void print_bytes(void *ptr, int size) {
  printf("Printing %d bytes\n", size);

  unsigned char *p = reinterpret_cast<unsigned char *>(ptr);
  int i;
  for (i = 0; i < size; i++) {

    if (i % 8 == 0)
      printf("\n");

    printf("%02hhX ", p[i]);
  }
  printf("\n\n");
}

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
  uint32_t *str;

  uint32_t ustr_size;
  uint32_t *ustr;
};

WasmEdge_Result Print(void *Data,
                      const WasmEdge_CallingFrameContext *CallFrameCxt,
                      const WasmEdge_Value *In, WasmEdge_Value *Out) {
  int32_t Val1 = WasmEdge_ValueGetI32(In[0]);
  uint32_t Offset = (uint32_t)WasmEdge_ValueGetI32(In[0]);

  // std::cout << Val1 << " " << Offset << std::endl; // 68956 68956

  WasmEdge_MemoryInstanceContext *MemCxt =
      WasmEdge_CallingFrameGetMemoryInstance(CallFrameCxt, 0);

  // auto mem = WasmEdge_MemoryInstanceGetPointer(MemCxt, Offset, 52);
  //  std::cout << "mem pointer --> " << (size_t)mem << std::endl;
  {
    uint8_t *hstruct_ptr =
        WasmEdge_MemoryInstanceGetPointer(MemCxt, Offset, 52);

    int32_t i32_sm = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 0));
    printf("The i32_sm: %u\n", i32_sm);
    int32_t i32_md = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 1));
    printf("The i32_md: %u\n", i32_md);
    int32_t i32_lg = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 2));
    printf("The i32_lg: %u\n", i32_lg);

    printf("\n\n-----------------------------\n\n");

    int32_t ni32_sm = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 3));
    printf("The ni32_sm: %d\n", ni32_sm);
    int32_t ni32_md = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 4));
    printf("The ni32_md: %d\n", ni32_md);
    int32_t ni32_lg = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 5));
    printf("The ni32_lg: %d\n", ni32_lg);

    printf("\n\n-----------------------------\n\n");

    uint32_t u32_sm = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 6));
    printf("The u32_sm: %u\n", u32_sm);
    uint32_t u32_md = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 7));
    printf("The u32_md: %u\n", u32_md);
    uint32_t u32_lg = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 4 * 8));
    printf("The u32_lg: %u\n", u32_lg);
  }

  printf("\n\n-----------------------------\n\n");

  // RECKLESS
  {
    uint8_t *mem_ptr =
        WasmEdge_MemoryInstanceGetPointer(MemCxt, 0, 128 * 1024 * 1024);
    uint32_t str_offset =
        *(reinterpret_cast<uint32_t *>(mem_ptr + Offset + 10 * 4));
    printf("The str_offset: %u\n", str_offset);
    char *str_ptr = reinterpret_cast<char *>(mem_ptr + str_offset);
    printf("The string: \n%s", str_ptr);
  }

  printf("\n\n-----------------------------\n\n");

  // a bit guessing
  {
    uint8_t *hstruct_ptr =
        WasmEdge_MemoryInstanceGetPointer(MemCxt, Offset, 52);
    uint32_t str_offset = *(reinterpret_cast<uint32_t *>(hstruct_ptr + 10 * 4));
    printf("The str_offset: %u\n", str_offset);
  }

  printf("\n\n-----------------------------\n\n");

  // carefull way
  {
    printf("The wasm struct ptr: %u\n", Offset);
    auto *hstruct_ptr = WasmEdge_MemoryInstanceGetPointer(MemCxt, Offset, 52);
    printf("The host struct ptr: %p\n", hstruct_ptr);
    // print_bytes(hstruct_ptr, 52);
    uint32_t str_offset = 0;
    WasmEdge_Result Res = WasmEdge_MemoryInstanceGetData(
        MemCxt, (uint8_t *)(&str_offset), Offset + 10 * 4, 4);
    if (!WasmEdge_ResultOK(Res))
      throw std::runtime_error("Res Error");
    printf("The str_offset: %u\n", str_offset);
    char *str_ptr = reinterpret_cast<char *>(
        WasmEdge_MemoryInstanceGetPointer(MemCxt, str_offset, 24));
    printf("The str_ptr: %p\n", str_ptr);
    printf("The string: \n%s", str_ptr);
  }

  //  Out[0] = WasmEdge_ValueGenI32(Val1 + Val2);
  return WasmEdge_Result_Success;
}

int main() {
  /* Create the VM context. */

  WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
  WasmEdge_ConfigureAddHostRegistration(ConfCxt,
                                        WasmEdge_HostRegistration_Wasi);
  WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(ConfCxt, NULL);

  /* Create the module instance. */
  WasmEdge_String ExportName = WasmEdge_StringCreateByCString("env");
  WasmEdge_ModuleInstanceContext *HostModCxt =
      WasmEdge_ModuleInstanceCreate(ExportName);

  /* {
    WasmEdge_Limit MemLimit = {
        .HasMax = true, .Shared = false, .Min = 16 * 128, .Max = 16 * 128};
    WasmEdge_MemoryTypeContext *HostMType = WasmEdge_MemoryTypeCreate(MemLimit);
    WasmEdge_MemoryInstanceContext *HostMemory =
        WasmEdge_MemoryInstanceCreate(HostMType);
    WasmEdge_String MemoryName = WasmEdge_StringCreateByCString("memory");
    WasmEdge_ModuleInstanceAddMemory(HostModCxt, MemoryName, HostMemory);
    WasmEdge_MemoryTypeDelete(HostMType);
  } */

  {
    enum WasmEdge_ValType ParamList[1] = {WasmEdge_ValType_I32};
    enum WasmEdge_ValType ReturnList[0];
    WasmEdge_FunctionTypeContext *PrintFType =
        WasmEdge_FunctionTypeCreate(ParamList, 1, ReturnList, 0);
    WasmEdge_FunctionInstanceContext *PrintFunc =
        WasmEdge_FunctionInstanceCreate(PrintFType, Print, NULL, 0);
    WasmEdge_FunctionTypeDelete(PrintFType);
    WasmEdge_String PrintFuncName = WasmEdge_StringCreateByCString("hostFunc");
    WasmEdge_ModuleInstanceAddFunction(HostModCxt, PrintFuncName, PrintFunc);
    WasmEdge_StringDelete(PrintFuncName);
  }

  WasmEdge_VMRegisterModuleFromImport(VMCxt, HostModCxt);
  /*   {

      enum WasmEdge_ValType ParamList[2] = {WasmEdge_ValType_I32,
                                            WasmEdge_ValType_I32};
      enum WasmEdge_ValType ReturnList[1] = {WasmEdge_ValType_I32};
      WasmEdge_FunctionTypeContext *PrintFType =
          WasmEdge_FunctionTypeCreate(ParamList, 2, ReturnList, 1);
      WasmEdge_FunctionInstanceContext *PrintFunc =
          WasmEdge_FunctionInstanceCreate(PrintFType, Alter, NULL, 0);
      WasmEdge_FunctionTypeDelete(PrintFType);
      WasmEdge_String PrintFuncName =
    WasmEdge_StringCreateByCString("alterVec");
      WasmEdge_ModuleInstanceAddFunction(HostModCxt, PrintFuncName, PrintFunc);
      WasmEdge_StringDelete(PrintFuncName);

      WasmEdge_VMRegisterModuleFromImport(VMCxt, HostModCxt);
    } */

  /* The parameters and returns arrays. */
  WasmEdge_Value Params[0];
  WasmEdge_Value Returns[1];
  /* Function name. */
  WasmEdge_String FuncName = WasmEdge_StringCreateByCString("test");
  /* Run the WASM function from file. */
  WasmEdge_Result Res = WasmEdge_VMRunWasmFromFile(
      VMCxt, "./module.wasm", FuncName, Params, 0, Returns, 1);

  if (WasmEdge_ResultOK(Res)) {
    // printf("Get the result: %d\n", WasmEdge_ValueGetI32(Returns[0]));
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_StringDelete(FuncName);
  WasmEdge_ModuleInstanceDelete(HostModCxt);
  return 0;
}
