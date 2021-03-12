#include <fstream>
#include <map>
#include <memory>

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
        token_consume = -29,
        token_not_equal_to = -30,
        token_boolean = -31,
        token_true = -32,
        token_false = -33,
        token_ampersand = -34,
        token_vertical_line = -35,
        token_left_square_brack = -36,
        token_right_square_brack = -37,
        token_increment = -38,
        token_decrement = -39,
        token_not = -40
    };
    map<string, token> KeywordRegistry;
    map<char, token> SymbolRegistry;
    string sourceName;
    std::ifstream source;
    double DoubleNum;
    int IntNum;
    string NumberString;
    string Identifier;
    int currentToken;
public:
    Lexer(string name) : sourceName(name)
    {
        source.open(sourceName);
        KeywordRegistry["int"] = token_int;
        KeywordRegistry["double"] = token_double;
        KeywordRegistry["void"] = token_void;
        KeywordRegistry["fn"] = token_fn;
        KeywordRegistry["return"] = token_return;
        KeywordRegistry["if"] = token_if;
        KeywordRegistry["else"] = token_else;
        KeywordRegistry["for"] = token_for;
        KeywordRegistry["consume"] = token_consume;
        KeywordRegistry["bool"] = token_boolean;
        KeywordRegistry["true"] = token_true;
        KeywordRegistry["false"] = token_false;
        SymbolRegistry[';'] = token_semi_colon;
        SymbolRegistry['*'] = token_mul_sym;
        SymbolRegistry['/'] = token_div_sym;
        SymbolRegistry['('] = token_left_paren;
        SymbolRegistry[')'] = token_right_paren;
        SymbolRegistry['{'] = token_left_curly_brac;
        SymbolRegistry['}'] = token_right_curly_brac;
        SymbolRegistry[','] = token_comma;
        SymbolRegistry['&'] = token_ampersand;
        SymbolRegistry['|'] = token_vertical_line;
        SymbolRegistry['['] = token_left_square_brack;
        SymbolRegistry[']'] = token_right_square_brack;
    }
    int getToken();
    int extractIdentifier();
    int extractNumber();
    int peekOneAhead(char sym, token tok);
    double getDoubleNum() { return DoubleNum; }
    int getIntNum() { return IntNum; }
    string getIdentifier() { return Identifier.c_str(); }
    int getNextToken() { return (currentToken = getToken()); }
    int getCurrentToken() { return currentToken; };
    void closeFile() { source.close(); }
    bool isTokenInt() { return currentToken == token_int; }
    bool isTokenDouble() { return currentToken == token_double; }
    bool isTokenIntNum() { return currentToken == token_int_num; }
    bool isTokenVoid() { return currentToken == token_void; }
    bool isTokenDoubleNum() { return currentToken == token_double_num; }
    bool isTokenIdentifier() { return currentToken == token_identifier; }
    bool isTokenAssignmentOp() { return currentToken == token_assignment_op; }
    bool isTokenSemiColon() { return currentToken == token_semi_colon; }
    bool isTokenAddSym() { return currentToken == token_add_sym; }
    bool isTokenSubSym() { return currentToken == token_sub_sym; }
    bool isTokenMulSym() { return currentToken == token_mul_sym; }
    bool isTokenDivSym() { return currentToken == token_div_sym; }
    bool isTokenLeftParen() { return currentToken == token_left_paren; }
    bool isTokenRightParen() { return currentToken == token_right_paren; }
    bool isAnyType() { return (isTokenInt() || isTokenDouble() || isTokenVoid() || isTokenBoolean()); }
    bool isTokenLeftCurlyBrace() { return currentToken == token_left_curly_brac; }
    bool isTokenRightCurlyBrace() { return currentToken == token_right_curly_brac; }
    bool isTokenFunctionKeyword() { return currentToken == token_fn; }
    bool isTokenReturnKeyword() { return currentToken == token_return; }
    bool isTokenConsume() { return currentToken == token_consume; }
    bool isTokenLessThan() { return currentToken == token_less_then; }
    bool isTokenGreaterThan() { return currentToken == token_greater_then; }
    bool isTokenLessThanEq() { return currentToken == token_less_than_eq; }
    bool isTokenGreaterThanEq() { return currentToken == token_greater_than_eq; }
    bool isTokenEqualTo() { return currentToken == token_equal_to; }
    bool isTokenIf() { return currentToken == token_if; }
    bool isTokenElse() { return currentToken == token_else; }
    bool isTokenFor() { return currentToken == token_for; }
    bool isTokenNotEqualTo() { return currentToken == token_not_equal_to; }
    bool isTokenComma() { return currentToken == token_comma; }
    bool isTokenBoolean() { return currentToken == token_boolean; }
    bool isTokenTrueValue() { return currentToken == token_true; }
    bool isTokenFalseValue() { return currentToken == token_false; }
    bool isTokenAmpersand() { return currentToken == token_ampersand; }
    bool isTokenVerticalLine() { return currentToken == token_vertical_line; }
    bool isTokenRightSquareBrack() { return currentToken == token_right_square_brack; }
    bool isTokenLeftSquareBrack() { return currentToken == token_left_square_brack; }
    bool isTokenIncrement() { return currentToken == token_increment; }
    bool isTokenDecrement() { return currentToken == token_decrement; }
    bool isTokenNot() { return currentToken == token_not; }
};
