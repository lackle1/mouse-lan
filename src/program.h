#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define SERVER_PORT "27015"

enum Role {
	SERVER = 0,
	CLIENT = 1
};

typedef struct {
	uint8_t type;
	char data[8];
} datapacket;

int run(enum Role role);
bool check_quit();