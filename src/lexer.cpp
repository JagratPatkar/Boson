#include "lexer.h"

using namespace Lexer;

Lexer::getToken(){
    char curChar;
    souce.get(curChar);
    if(curChar == EOF) return token_eof;
    if(isdigit(curChar)) 
}