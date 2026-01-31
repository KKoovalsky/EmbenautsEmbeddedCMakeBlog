#include <stdio.h>
#include "mylib.h"

int main(void) {
    printf("Platform: %s\n", mylib_get_platform());
    printf("2 + 3 = %d\n", mylib_add(2, 3));
    return 0;
}
