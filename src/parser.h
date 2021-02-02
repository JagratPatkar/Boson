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
    std::unique_ptr<Expression> ParseVariableDeclaration();
    std::unique_ptr<Expression> LogError(const char*);
    std::unique_ptr<Expression> LogTypeError(int,int);
    void parse();
    void driver();
};