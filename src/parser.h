#include "ast.h"
#include <memory>
#include "lexer.h"
using namespace AST;
using namespace std;

class Parser{
    Lexer lexer; 
    public:
    Parser(string Name) : lexer(Lexer(Name)) {}
    std::unique_ptr<AST::Expression> ParseExpression();
    std::unique_ptr<AST::Statement> ParseStatement();
    std::unique_ptr<AST::Statement> ParseCompundStatement();
    std::unique_ptr<AST::Expression> ParsePrimary();
    std::unique_ptr<AST::Expression> ParseIntNum();
    std::unique_ptr<AST::Expression> ParseIdentifier();
    std::unique_ptr<AST::Expression> ParseDoubleNum(); 
    std::unique_ptr<AST::Statement> ParseVariableDeclarationStatement();
    std::unique_ptr<AST::Expression> LogExpressionError(const char*);
    std::unique_ptr<AST::Statement> LogStatementError(const char*);
    std::unique_ptr<AST::Expression> LogTypeError(int,int);
    void parse();
    void driver();
};