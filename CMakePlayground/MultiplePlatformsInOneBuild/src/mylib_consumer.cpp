#include <stdio.h>

extern "C" int mylib_add(int a, int b);
extern "C" const char* mylib_get_platform(void);

int main(void) {
    printf("Platform: %s\n", mylib_get_platform());
    printf("2 + 3 = %d\n", mylib_add(2, 3));
    return 0;
}
