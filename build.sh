#!/usr/bin/env bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=TC-mingw.cmake 
cmake --build build --config Release 
echo finished
