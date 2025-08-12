.PHONY: all clean

program: build/mouse_lan

#
# Program
#
build/mouse_lan: build/main.o build/tools_linux.o
	gcc -g -o build/mouse_lan build/main.o build/tools_linux.o -lX11

build/main.o: always src/main.c
	gcc -g -o build/main.o src/main.c -c

build/tools_linux.o: always src/tools_linux.c
	gcc -g -o build/tools_linux.o src/tools_linux.c -c

#
# Always
#
always:
	mkdir -p build

#
# Clean
#
clean:
	rm -rf build