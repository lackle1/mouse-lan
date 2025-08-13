#pragma once

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int Run();