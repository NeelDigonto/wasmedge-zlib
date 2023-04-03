em++ src/module.cpp -o module.wasm -O2 -sSTANDALONE_WASM -sSIDE_MODULE && g++ src/host.cpp -o host -O2 -lwasmedge && ./host

em++ module.cpp -O2 -o module.wasm -sSTANDALONE_WASM -sWARN_ON_UNDEFINED_SYMBOLS=0 -sIMPORTED_MEMORY -sINITIAL_MEMORY=128MB -sMAXIMUM_MEMORY=128MB -sALLOW_MEMORY_GROWTH=0 && g++ -O2 host.cpp -o host -lwasmedge && ./host