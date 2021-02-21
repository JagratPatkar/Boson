#include <stdio.h>

extern "C" {
    int start();
}

extern "C" void printchar(int ascii){
    putchar((char)ascii);
}

extern "C" void printint(int val){
    printf("%d\n",val);
}

int main(){
    start();
    return 0;   
}