@echo off
mkdir build
cl /Fobuild\ /Isrc src/main.c src/win/program.c src/win/server.c src/win/client.c /link user32.lib Ws2_32.lib Iphlpapi.lib /out:build/mouse_lan.exe