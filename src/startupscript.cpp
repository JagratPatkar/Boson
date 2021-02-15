#include <stdio.h>

extern "C" {
    int start();
}

extern "C" void printchar(int ascii){
    putchar((char)ascii);
}

int main(){
    start();
    return 0;   
}