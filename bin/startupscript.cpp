#include <stdio.h>

extern "C"
{
    void start();
}

extern "C" void printchar(int ascii)
{
    putchar((char)ascii);
}

extern "C" void printint(int val)
{
    printf("%d\n", val);
}

extern "C" void nextLine()
{
    putchar('\n');
}

int main()
{
    start();
    return 0;
}