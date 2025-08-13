#ifdef _WIN32 //  Just to prevent windows.h error

#include "program.h"
#include <windows.h>

bool GetDisplayDimensions(int *width, int *height) {
    *width = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);
    
    return true;
}

#endif