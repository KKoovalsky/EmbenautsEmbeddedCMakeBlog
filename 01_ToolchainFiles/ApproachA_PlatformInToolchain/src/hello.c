void hello_from_c(void)
{
    // This function will be compiled with platform flags from the toolchain
    volatile int x = 42;
    (void)x;
}
