#ifdef __linux__

#include "program.h"
#include <X11/Xlib.h>

bool GetDisplayDimensions(Display *dpy, int *width, int *height) {
	int snum = DefaultScreen(dpy);
	*width = DisplayWidth(dpy, snum);
	*height = DisplayHeight(dpy, snum);

	return true;
}

int Run(enum Role role) {
	Display *dpy;
	if (!(dpy = XOpenDisplay(NULL))) {
		printf("Could not open display %s", XDisplayName(NULL));
		return -1;
	}

	int width, height;
	GetDisplayDimensions(dpy, &width, &height);
	printf("Dimensions: %d x %d\n", width, height);

	int x, y, winX, winY;
	unsigned int mask;
	Window window = DefaultRootWindow(dpy);
	Window what;
	Window child;
	if (!XQueryPointer(dpy, window, &what, &child, &x, &y, &winX, &winY, &mask)) {
		printf("Pointer query failed");
	}
	printf("\nx => %d, y => %d\n", x, y);

	XCloseDisplay(dpy);

	return 0;
}

#endif