@echo off

set mode=%1

cmake --preset custom_configure
cd out/build/custom_configure
ninja

if "%mode%"=="server" (
    .\server.exe
) else if "%mode%"=="client" (
    .\client.exe
) else if "%mode%"=="build" (
    echo building
) else (
    .\favali.exe
)

cd ../../..
