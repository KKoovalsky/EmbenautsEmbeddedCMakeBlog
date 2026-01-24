extern "C" void hello_from_c(void);
extern "C" void hello_from_cpp(void);

int main()
{
    hello_from_c();
    hello_from_cpp();
    return 0;
}
