#include "lexer.h"
using namespace std;
int Lexer::getToken(){
    char curChar = ' ';
    
    while(isspace(curChar) && !source.eof())
        source.get(curChar);

    if(isdigit(curChar)){
        bool isDouble = false;
        NumberString = "";
        NumberString += curChar;
        char dummy = source.peek();
        while(dummy == '.' || isdigit(dummy)){
            source.get(curChar);
            if(curChar == '.') isDouble = true;
            NumberString += curChar;
            dummy = source.peek();
        }
        if(isDouble){
            DoubleNum = strtod(NumberString.c_str(), 0);
            return token_double_num;
        }
        IntNum = stoi(NumberString.c_str(),0);
        return token_int_num;
    }

    if(isalpha(curChar)){
        Identifier = "";
        Identifier += curChar;
        char dummy = source.peek();
        while(isalnum(dummy)){
            source.get(curChar);
            Identifier += curChar;
            dummy = source.peek();
        }
        if(Identifier == "int")
            return token_int;
        if(Identifier == "double")
            return token_double;
        return token_identifier;
    }

    if(source.eof()) return token_eof;
    printf("Unknown token \n");
    exit(-1);
}

int Lexer::getNextToken() {
    return (currentToken = getToken());
}

int Lexer::getCurrentToken() {
    return currentToken;
}