#if defined(_WIN32)
	#define PLATFORM "Windows"
#elif defined(__linux__)
	#define PLATFORM "Linux"
#else
	#define PLATFORM "Unknown"
#endif

#include <stdio.h>
#include "program.h"

int main(int argc, char **argv) {
    //printf("PLATFORM: %s\n", PLATFORM);
	if (PLATFORM == "Unknown") {
		printf("ERROR: Unsupported OS");
		return -1;
	}

	if (argc < 3 || (argv[1][0] != '0' && argv[1][0] != '1')) {
		printf("Syntax: %s <role> <parameter>\n<role> - 0: server, 1: client\n<parameter> - update speed (ms) for server, or IPv4 address of server for client", argv[0]);
		return -2;
	}
    
	int role = argv[1][0] - '0';

	run(role, argv[2]);

	return 0;
}

