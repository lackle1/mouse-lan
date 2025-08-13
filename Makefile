.PHONY: all clean

program: build/mouse_lan

#
# Program
#
build/mouse_lan: build/main.o build/program_linux.o
	gcc -g -o build/mouse_lan build/main.o build/program_linux.o -lX11

build/main.o: always src/main.c
	gcc -g -o build/main.o src/main.c -c

build/program_linux.o: always src/program_linux.c
	gcc -g -o build/program_linux.o src/program_linux.c -c

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