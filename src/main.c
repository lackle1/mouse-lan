#if defined(_WIN32)
	#define PLATFORM "Windows"
	#include <windows.h>
#elif defined(__linux__)
	#define PLATFORM "Linux"
	#include <X11/Xlib.h>
#else
	#define PLATFORM "Unknown"
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "program.h"

enum Role {
	SERVER = 0,
	CLIENT = 1
};

int main(int argc, char **argv) {
    printf("PLATFORM: %s\n", PLATFORM);
	if (PLATFORM == "Unknown") {
		printf("ERROR: Unsupported OS");
		return -1;
	}

	if (argc < 2 || (argv[1][0] != '0' && argv[1][0] != '1')) {
		printf("Syntax: %s <role>\n0: server, 1: client\n", argv[0]);
		return -2;
	}
    
	enum Role role = argv[1][0] - '0';

	Run();

	return 0;
}

