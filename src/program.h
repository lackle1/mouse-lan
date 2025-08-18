#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define ROLE_TYPE_SERVER	0
#define ROLE_TYPE_CLIENT	1

#define SERVER_PORT			"27015"

#define DP_SIZE_BITS		64
#define DP_SIZE_BYTES		8
#define DP_TYPE_MOUSE_INFO	1
#define DP_TYPE_MSG			2

#define MOUSE_POS_MAX		65535 // uint16 max

#define MOUSE_BTN_LEFT		0b0000001
#define MOUSE_BTN_MIDDLE	0b0000010
#define MOUSE_BTN_RIGHT		0b0000100

#pragma pack(push, DP_SIZE_BYTES)
typedef struct data_packet {
	uint8_t type;
	char data[7];
} dp;

typedef struct data_packet_mouse_info {
	uint8_t type;
	uint16_t x;
	uint16_t y;
	uint8_t btn_flags;
	char zero[2];
} dp_mouse_info;
#pragma pack(pop)

// Can't be bothered changing things to use this
// typedef struct vec2 {
// 	uint16_t x;
// 	uint16_t y;
// } vec2;

extern int scr_width, scr_height;

int run(int role, char* ip);
bool check_quit();