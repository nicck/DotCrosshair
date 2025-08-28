#!/bin/bash

zig cc -target x86_64-windows-gnu -s -municode -O3 -Wl,--subsystem,windows -e wWinMain WinMain.c -lole32 -o DotCrosshair.exe
zig cc -target x86_64-windows-gnu -s -municode -O3 -shared -e DllMain DllMain.c -lgdi32 -o DotCrosshair.dll
upx --best --ultra-brute DotCrosshair.dll DotCrosshair.exe
