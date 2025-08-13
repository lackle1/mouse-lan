#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

enum Role {
	SERVER = 0,
	CLIENT = 1
};

int Run(enum Role role);