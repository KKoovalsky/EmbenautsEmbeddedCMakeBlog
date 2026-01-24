extern "C" void hello_from_cpp(void)
{
    // This function will be compiled with platform flags from the toolchain
    volatile int y = 100;
    (void)y;
}
