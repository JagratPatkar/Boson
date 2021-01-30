#include "lexer.h"
using namespace std;
int Lexer::getToken(){
    char curChar;
    source.get(curChar);
    
    while(isspace(curChar)) source.get(curChar);

    if(isdigit(curChar)){
        char isDouble;
        NumberString = "";
        NumberString += curChar;
        char dummy = source.peek();
        while(dummy == '.' || isdigit(dummy)){
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

    if(source.eof()) return token_eof;
    printf("Unknown token \n");
    exit(-1);
}