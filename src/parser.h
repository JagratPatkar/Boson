#include "ast.h"
#include <memory>
#include <map>
#include "lexer.h"
using namespace AST;
using namespace std;

class Parser{
    Lexer lexer; 
    map<string,Types> SymbolTable;
    map<char,int> OperatorPrecedence;
    public:
    Parser(string Name) : lexer(Lexer(Name)) {
        OperatorPrecedence['+'] = 10;
        OperatorPrecedence['-'] = 10;
        OperatorPrecedence['*'] = 40;
        OperatorPrecedence['/'] = 40;
    }
    std::unique_ptr<AST::Expression> ParseExpression();
    std::unique_ptr<AST::Expression> ParseParen();
    std::unique_ptr<AST::Statement> ParseStatement();
    std::unique_ptr<CompoundStatement> ParseCompoundStatement();
    std::unique_ptr<AST::Expression> ParsePrimary();
    std::unique_ptr<AST::Expression> ParseIntNum();
    std::unique_ptr<AST::Expression> ParseIdentifier();
    std::unique_ptr<AST::Expression> ParseDoubleNum(); 
    std::unique_ptr<AST::Expression> ParseBinOP(int,unique_ptr<Expression>);
    std::unique_ptr<AST::Statement> ParseVariableDeclarationStatement();
    unique_ptr<AST::Statement> ParseVariableAssignmentStatement();
    unique_ptr<AST::Statement> ParseReturnStatement();
    std::unique_ptr<FunctionSignature> ParseFunctionSignature();
    std::unique_ptr<FunctionDefinition> ParseFunctionDefinition();
    std::unique_ptr<AST::Expression> LogExpressionError(const char*);
    std::unique_ptr<AST::Statement> LogStatementError(const char*);
    std::unique_ptr<AST::Expression> LogTypeError(int,int);
    unique_ptr<FunctionSignature> LogFuncSigError(const char*);
    unique_ptr<FunctionDefinition> LogFuncDefError(const char*);
    unique_ptr<CompoundStatement> LogCompStatementError(const char*);
    int getOperatorPrecedence();
    void addVariable(const string&,Types);
    bool doesVariableExist(const string&);
    Types getVariableType(const string&);
    BinOps returnBinOpsType();
    void parse();
    void driver();
};