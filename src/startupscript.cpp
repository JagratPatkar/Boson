#include <stdio.h>

extern "C" {
    int start();
}

extern "C" void printchar(int ascii){
    putchar((char)ascii);
}

int main(){
    printf("Start Retured :- %d \n", start());
    return 0;   
}