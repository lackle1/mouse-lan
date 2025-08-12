mkdir build
cl /Fobuild\ src/main.c src/tools_windows.c /link user32.lib /out:build/mouse_lan.exe
pause