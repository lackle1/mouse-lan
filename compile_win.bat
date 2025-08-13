mkdir build
cl /Fobuild\ src/main.c src/program_win.c /link user32.lib /out:build/mouse_lan.exe