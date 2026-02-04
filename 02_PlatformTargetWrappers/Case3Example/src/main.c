/**
 * main.c - Minimal embedded application
 */

#include <stdint.h>

extern int mylib_getValue(void);

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile("nop");
    }
}

int main(void)
{
    volatile uint32_t counter = mylib_getValue();

    while (1) {
        counter++;
        delay(100000);
    }

    return 0;
}

void _c_int00(void)
{
    main();
    while (1) {}
}

__attribute__((section(".vectors")))
void (*const vector_table[])(void) = {
    _c_int00,
};
