// -------------------------------------------------------------------------- //
// glx.cpp
// -------------------------------------------------------------------------- //
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <iostream>

// -------------------------------------------------------------------------- //
// main
// -------------------------------------------------------------------------- //
int main(int, char**)
{
    // Display
    Display* dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        std::cout << "error: cannot open display" << std::endl;
        return EXIT_FAILURE;
    }

    // Extensions
    int n = 0;
    char** extlist = XListExtensions(dpy, &n);
    XCloseDisplay(dpy);
    if (extlist == NULL) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; ++i) {
        if (strcmp(extlist[i], "GLX") == 0) {
            std::cout << "GLX extension found" << std::endl;
            return EXIT_SUCCESS;
        }
    }

    std::cout << "GLX extension not found" << std::endl;
    return EXIT_FAILURE;
}
