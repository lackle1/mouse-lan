#ifdef _WIN32 //  Just to prevent windows.h error

#include "program.h"
#include <windows.h>

#define HORIZONTAL_SHIFT 8;

int RunServer(int scrWidth, int scrHeight) {
    POINT p;
    HWND hwnd = GetForegroundWindow();
    RECT winRect;
    int x, y;
    float normalisedX, normalisedY;

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

        GetWindowRect(hwnd, &winRect);
        x = p.x + winRect.left + HORIZONTAL_SHIFT;
        y = p.y + winRect.top;
        
        normalisedX = (float)x / scrWidth;
        normalisedY = (float)y / scrHeight;
        
        printf("\rMouse Position: %f, %f                 ", normalisedX, normalisedY); // + a bunch of spaces to get rid of any numbers leftover from before

        Sleep(50);
    }
}

int Run(enum Role role) {
    int scrWidth = GetSystemMetrics(SM_CXSCREEN);
    int scrHeight = GetSystemMetrics(SM_CYSCREEN);

    RunServer(scrWidth, scrHeight);

    return 0;
}

#endif