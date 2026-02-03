/**
 * main.c - Minimal embedded application
 */

#include <stdint.h>

extern int mylib_getSomeValue(void);

/* Simple delay */
static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile("nop");
    }
}

/* Main application */
int main(void)
{
    volatile uint32_t counter = mylib_getSomeValue();

    while (1) {
        counter++;
        delay(100000);
    }

    return 0;
}

/* Reset handler - called on startup */
void _c_int00(void)
{
    /* Call main */
    main();

    /* Hang if main returns */
    while (1) {}
}

/* Vector table */
__attribute__((section(".vectors")))
void (*const vector_table[])(void) = {
    _c_int00,  /* Reset */
};
