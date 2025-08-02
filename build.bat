@echo off
cmake -B build
::cmake --build build --config Debug
cmake --build build --config Release
pause