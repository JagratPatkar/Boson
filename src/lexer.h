#include <fstream>
using namespace std;
class Lexer
{
    enum token
    {
        token_int_num = -2,
        token_double_num = -3,
        token_eof = -1,
        token_int = -4,
        token_double = -5,
        token_assignment_op = -6,
        token_identifier = -7,
        token_semi_colon = -8,
        token_add_sym = -9,
        token_sub_sym = -10,
        token_div_sym = -11,
        token_mul_sym = -12,
        token_left_paren = -13,
        token_right_paren = -14,
        token_void = -15,
        token_fn = -16,
        token_left_curly_brac = -17,
        token_right_curly_brac = -18,
        token_return = -19,
        token_comma = -20,
        token_less_then = -21,
        token_greater_then = -22,
        token_less_than_eq = -23,
        token_greater_than_eq = -24,
        token_equal_to = -25,
        token_if = -26,
        token_else = -27,
        token_for = -28,
        token_consume = -29
    };
    string sourceName;
    std::ifstream source;
    double DoubleNum;
    int IntNum;
    string NumberString;
    string Identifier;
    int currentToken;
public:
    Lexer(string name) : sourceName(name){
        source.open(sourceName);
    }
    int getToken();
    double getDoubleNum(){ return DoubleNum; }
    int getIntNum(){ return IntNum; }
    string getIdentifier() {return Identifier.c_str();}
    int getNextToken(){ return (currentToken = getToken()); }
    int getCurrentToken(){ return currentToken; };
    void closeFile() { source.close(); }
    bool isTokenInt(){ return currentToken == token_int; }
    bool isTokenDouble(){ return currentToken == token_double; }
    bool isTokenIntNum();
    bool isTokenVoid();
    bool isTokenDoubleNum();
    bool isTokenIdentifier();
    bool isTokenAssignmentOp();
    bool isTokenSemiColon();
    bool isTokenAddSym();
    bool isTokenSubSym();
    bool isTokenMulSym();
    bool isTokenDivSym();
    bool isTokenLeftParen();
    bool isTokenRightParen();
    bool isAnyType();
    bool isTokenLeftCurlyBrace();
    bool isTokenRightCurlyBrace();
    bool isTokenFunctionKeyword();
    bool isTokenReturnKeyword();
    bool isTokenConsume(){return currentToken == token_consume; }
    bool isTokenLessThan(){ return currentToken == token_less_then; }
    bool isTokenGreaterThan(){ return currentToken == token_greater_then; }
    bool isTokenLessThanEq(){ return currentToken == token_less_than_eq; }
    bool isTokenGreaterThanEq(){ return currentToken == token_greater_than_eq; }
    bool isTokenEqualTo(){ return currentToken == token_equal_to; }
    bool isTokenIf(){ return currentToken == token_if; }
    bool isTokenElse(){ return currentToken == token_else; }
    bool isTokenFor(){ return currentToken == token_for; }
    bool isTokenComma();
    int getVoidToken();
};
