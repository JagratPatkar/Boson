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

    if(curChar == '=')
        return token_assignment_op;

    if(curChar == ';')
        return token_semi_colon;

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

bool Lexer::isTokenInt(){
    if(currentToken == token_int) return true;
    return false;
}

bool Lexer::isTokenDouble(){
    if(currentToken == token_double) return true;
    return false;
}

bool Lexer::isTokenIdentifier(){
    if(currentToken == token_identifier) return true;
    return false;
}

bool Lexer::isTokenIntNum(){
    if(currentToken == token_int_num) return true;
    return false;
}

bool Lexer::isTokenDoubleNum(){
    if(currentToken == token_double_num) return true;
    return false;
}

bool Lexer::isTokenAssignmentOp(){
    if(currentToken == token_assignment_op) return true;
    return false;
}

bool Lexer::isTokenSemiColon(){
    if(currentToken == token_semi_colon) return true;
    return false;
}