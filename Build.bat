@echo off
cd "%~dp0"
zig cc -s -municode -O3 -Wl,--subsystem,windows -e wWinMain WinMain.c -lOle32 -o DotCrosshair.exe
zig cc -s -municode -O3 -shared -e DllMain DllMain.c -lGdi32 -o DotCrosshair.dll
upx.exe --best --ultra-brute DotCrosshair.dll DotCrosshair.exe>nul 2>&1