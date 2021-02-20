#include "lexer.h"
using namespace std;
int Lexer::getToken()
{
    char curChar = ' ';

    while (isspace(curChar) && !source.eof())
        source.get(curChar);
    if (isdigit(curChar))
    {
        NumberString = "";
        NumberString += curChar;
        return extractNumber();
    }
    if (isalpha(curChar))
    {
        Identifier = "";
        Identifier += curChar;
        return extractIdentifier();
    }
    auto it = SymbolRegistry.find(curChar);
    if (it != SymbolRegistry.end())
        return it->second;
    int tmp;
    if (curChar == '=')
        return (tmp = peekOneAhead('=', token_equal_to)) ? tmp : token_assignment_op;
    if (curChar == '<')
        return (tmp = peekOneAhead('=', token_less_than_eq)) ? tmp : token_less_then;
    if (curChar == '>')
        return (tmp = peekOneAhead('=', token_greater_than_eq)) ? tmp : token_greater_then;
    if (curChar == '!')
    {
        if ((tmp = peekOneAhead('=', token_less_than_eq)))
            return tmp;
    }
    

    if (source.eof())
        return token_eof;
    printf("Unknown token \n");
    exit(-1);
}

int Lexer::extractNumber()
{
    bool isDouble = false;
    char curChar;
    char dummy = source.peek();
    while (dummy == '.' || isdigit(dummy))
    {
        source.get(curChar);
        if (curChar == '.')
            isDouble = true;
        NumberString += curChar;
        dummy = source.peek();
    }
    if (isDouble)
    {
        DoubleNum = strtod(NumberString.c_str(), 0);
        return token_double_num;
    }
    IntNum = stoi(NumberString.c_str(), 0);
    return token_int_num;
}

int Lexer::extractIdentifier()
{
    char curChar;
    char dummy = source.peek();
    while (isalnum(dummy))
    {
        source.get(curChar);
        Identifier += curChar;
        dummy = source.peek();
    }
    auto it = KeywordRegistry.find(Identifier);
    if (it != KeywordRegistry.end())
        return it->second;
    else
        return token_identifier;
}

int Lexer::peekOneAhead(char sym, token tok)
{
    if (source.peek() == sym)
    {
        source.get();
        return tok;
    }
    return 0;
}