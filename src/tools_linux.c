#include "tools.h"

bool GetDisplayDimensions(int *width, int *height) {
	Display *display;
	if (!(display = XOpenDisplay(NULL))) {
		printf("Could not open display %s", XDisplayName(NULL));
		return false;
	}

	int snum = DefaultScreen(display);
	*width = DisplayWidth(display, snum);
	*height = DisplayHeight(display, snum);

	XCloseDisplay(display);

	return true;
}