#include "ast.h"
#include <stdio.h>
#include <memory>
#include "lexer.h"
using namespace AST;
using namespace std;

class Parser{
    Lexer lexer; 
    public:
    Parser(string Name) : lexer(Lexer(Name)) {}
    std::unique_ptr<Expression> ParseExpression();
    std::unique_ptr<Expression> ParsePrimary();
    std::unique_ptr<Expression> ParseIntNum();
    std::unique_ptr<Expression> ParseIdentifier();
    std::unique_ptr<Expression> ParseDoubleNum(); 
    void parse();
    void driver();
};