#include "header.h"

int main (int argc, char *argv[]) {
    #if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
        return gst_macos_main (tutorial_main, argc, argv, NULL);
    #else
        return localhost_pipeline (argc, argv);
    #endif
    return 0;
}