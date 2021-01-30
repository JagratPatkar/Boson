#include "lexer.h"
using namespace std;
int Lexer::getToken(){
    char curChar;
    source.get(curChar);
    
    if(isdigit(curChar)){
        char isDouble;
        NumberString += curChar;
        while(source.peek() == '.' || isdigit(source.peek())){
            source.get(curChar);
            if(curChar == '.') isDouble = true;
            NumberString += curChar;
        }
        if(isDouble){
            DoubleNum = strtod(NumberString.c_str(), 0);
            return token_double_num;
        }
        IntNum = stoi(NumberString.c_str(),0);
        return token_int_num;
    }

    if(curChar == source.eof()) return token_eof;

    printf("Unknown token \n");
    exit(-1);
}