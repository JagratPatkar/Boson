#include <stdio.h>

int m1(){ return 10; }

int m[5] = {10,20,m1(),40,50};

void f(int a[]){
    a[1] = 10000;
}

int main(){
    bool i = true;
    int k[10] = {10,20+20,30,40,50,60,70,80,90,100};
    k[0] = 100;
    int  d = k[0] + 10;
    f(k);
    return 0;
}