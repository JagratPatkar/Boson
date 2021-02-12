#include <stdio.h>

int main(){
    bool j = false;
    bool k =  true || false && (j && j) || (j && true);
    if(true){
        printf("in if");
    }
    else{
        printf("in else");
    }
    int i = 10;

    return 0;
}