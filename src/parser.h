#include "ast.h"
#include <memory>
#include <map>
#include <utility>
#include "lexer.h"
using namespace AST;
using namespace std;

class Parser{
    Lexer lexer; 
    map<string,Types> SymbolTable;
    map<string,Types> SymbolTableLocal;
    map<string,pair<vector<Types>,Types>> FuncSymTab;
    map<string,int> OperatorPrecedence;
    public:
    Parser(string Name) : lexer(Lexer(Name)) {
        OperatorPrecedence["+"] = 10;
        OperatorPrecedence["-"] = 10;
        OperatorPrecedence["*"] = 40;
        OperatorPrecedence["/"] = 40;
        OperatorPrecedence["<"] = 10;
        OperatorPrecedence["<="] = 10;
        OperatorPrecedence[">"] = 10;
        OperatorPrecedence[">="] = 10;
        OperatorPrecedence["=="] = 10; 
        OperatorPrecedence["!="] = 10; 
    }
    std::unique_ptr<AST::Expression> ParseExpression();
    std::unique_ptr<AST::Expression> ParseParen();
    std::unique_ptr<AST::Statement> ParseStatement();
    std::unique_ptr<AST::Statement> ParseStatementIdentifier();
    std::unique_ptr<CompoundStatement> ParseCompoundStatement();
    std::unique_ptr<Statement> ParseIfElseStatement();
    std::unique_ptr<Statement> ParseForStatement();
    std::unique_ptr<AST::Expression> ParsePrimary();
    std::unique_ptr<FunctionSignature> ParseConsume();
    std::unique_ptr<AST::Expression> ParseCallExpression(const string&);
    std::unique_ptr<AST::Expression> ParseVariable(const string&);
    std::unique_ptr<AST::Expression> ParseIntNum();
    std::unique_ptr<AST::Expression> ParseIdentifier();
    std::unique_ptr<AST::Expression> ParseDoubleNum(); 
    std::unique_ptr<AST::Expression> ParseBinOP(int,unique_ptr<Expression>);
    std::unique_ptr<AST::Statement> ParseVariableDeclarationStatement();
    std::unique_ptr<AST::Statement> ParseLocalVariableDeclarationStatement();
    unique_ptr<AST::Statement> ParseVariableAssignmentStatement(const string&);
    unique_ptr<AST::VariableAssignment> VariableAssignmentStatementHelper(const string&);
    unique_ptr<AST::Statement> ParseCallStatement(const string&);
    unique_ptr<AST::Statement> ParseReturnStatement();
    std::unique_ptr<FunctionSignature> ParseFunctionSignature();
    std::unique_ptr<FunctionDefinition> ParseFunctionDefinition();
    void LogError(const char*);
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
    void addVariableLocally(const string&,Types);
    bool doesVariableExistLocally(const string&);
    Types getVariableTypeLocally(const string&);
    void clearVariablesLocally();
    BinOps returnBinOpsType();
    bool isExpression();
    bool doesFunctionExist(const string&);
    bool doesStartExist();
    void parse();
    void driver();
};