#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include "program.h"
#include <windows.h>

#include <winsock2.h>

#define HORIZONTAL_SHIFT 8

int blah(int scr_width, int scr_height) {
    POINT p;
    HWND hwnd = GetForegroundWindow();
    RECT win_rect;
    int x, y;
    float normalised_x, normalised_y;

    printf("Tracking mouse position. CTRL+C to exit.\n");
    while(true) {
        if (!GetCursorPos(&p)) {
            printf("\nError getting cursor position: %d\n", GetLastError());
            continue;
        }

        if (!ScreenToClient(hwnd, &p)) {
            printf("\nScreen to client failed. Error: %d\n", GetLastError());
            continue;
        }

        GetWindowRect(hwnd, &win_rect);
        x = p.x + win_rect.left + HORIZONTAL_SHIFT;
        y = p.y + win_rect.top;
        
        normalised_x = (float)x / scr_width;
        normalised_y = (float)y / scr_height;
        
        printf("\rMouse Position: %f, %f       ", normalised_x, normalised_y); // + a bunch of spaces to get rid of any numbers leftover from before

        Sleep(10);
    }
}

bool check_quit() {
    if (GetAsyncKeyState('q')) {
        printf("Quitting...\n");
        return true;
    }

    return false;
}

int run(enum Role role) {
    int scr_width = GetSystemMetrics(SM_CXSCREEN);
    int scr_height = GetSystemMetrics(SM_CYSCREEN);

    WSADATA wsa_data;
    int i_res = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if (i_res != 0) {
        printf("WSA initialisation failed. RESULT: %d\n", i_res);
        return -1;
    }

    int res = 0;
    if (role == SERVER) {
        res = run_server();
    }
    else{
        res = run_client("192.168.1.18");
    }
    return res;
}

#endif