#include <stdio.h>
#include <wasmedge/wasmedge.h>

WasmEdge_Result Print(void *Data,
                      const WasmEdge_CallingFrameContext *CallFrameCxt,
                      const WasmEdge_Value *In, WasmEdge_Value *Out) {
  int32_t Val1 = WasmEdge_ValueGetI32(In[0]);
  // int32_t Val2 = WasmEdge_ValueGetI32(In[1]);
  printf("Host function \"Struct P\": %dn", Val1);
  //  Out[0] = WasmEdge_ValueGenI32(Val1 + Val2);
  return WasmEdge_Result_Success;
}

/* Host function body definition. */
WasmEdge_Result Alter(void *Data,
                      const WasmEdge_CallingFrameContext *CallFrameCxt,
                      const WasmEdge_Value *In, WasmEdge_Value *Out) {
  int32_t Val1 = WasmEdge_ValueGetI32(In[0]);
  int32_t Val2 = WasmEdge_ValueGetI32(In[1]);
  printf("Host function \"Add\": %d + %d\n", Val1, Val2);
  Out[0] = WasmEdge_ValueGenI32(1);
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

  {

    enum WasmEdge_ValType ParamList[1] = {WasmEdge_ValType_I32};
    enum WasmEdge_ValType ReturnList[0];
    WasmEdge_FunctionTypeContext *PrintFType =
        WasmEdge_FunctionTypeCreate(ParamList, 1, ReturnList, 0);
    WasmEdge_FunctionInstanceContext *PrintFunc =
        WasmEdge_FunctionInstanceCreate(PrintFType, Print, NULL, 0);
    WasmEdge_FunctionTypeDelete(PrintFType);
    WasmEdge_String PrintFuncName = WasmEdge_StringCreateByCString("print");
    WasmEdge_ModuleInstanceAddFunction(HostModCxt, PrintFuncName, PrintFunc);
    WasmEdge_StringDelete(PrintFuncName);

    WasmEdge_VMRegisterModuleFromImport(VMCxt, HostModCxt);
  }

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
    printf("Get the result: %d\n", WasmEdge_ValueGetI32(Returns[0]));
  } else {
    printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
  }

  /* Resources deallocations. */
  WasmEdge_VMDelete(VMCxt);
  WasmEdge_StringDelete(FuncName);
  WasmEdge_ModuleInstanceDelete(HostModCxt);
  return 0;
}
