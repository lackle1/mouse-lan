#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "program.h"
#include <windows.h>

#include <winsock2.h>

#define HORIZONTAL_SHIFT 8

int scr_width, scr_height;

void process_packet(dp *packet) {
    switch (packet->type) {
        case DP_TYPE_MOUSE_INFO:
            handle_mouse_info((struct dp_mouse_info*)packet);
            break;
        case DP_TYPE_MSG:
            printf("Incoming message: %s\n", packet->data);
            break;
        default:
            printf("Unknown packet received...\n");
            break;
    }
}

bool check_quit() {
    if (GetAsyncKeyState('Q') & 0x8001) {
        printf("Quitting...\n");
        return true;
    }

    return false;
}

int run(int role, char *parameter) {
    scr_width = GetSystemMetrics(SM_CXSCREEN);
    scr_height = GetSystemMetrics(SM_CYSCREEN);

    WSADATA wsa_data;
    int i_res = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (i_res != 0) {
        printf("WSA initialisation failed. RESULT: %d\n", i_res);
        return -1;
    }

    int res = 0;
    if (role == ROLE_TYPE_SERVER) {
        res = run_server(parameter);
    }
    else{
        res = run_client(parameter);
    }
    return res;
}

#endif