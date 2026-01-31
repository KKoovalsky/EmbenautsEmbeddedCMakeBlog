#include "mylib.h"

int mylib_add(int a, int b) {
    return a + b;
}

const char* mylib_get_platform(void) {
#if defined(PLATFORM_AM243X)
    return "am243x";
#elif defined(PLATFORM_TMS570)
    return "tms570";
#else
    return "unknown";
#endif
}
