#include "ast.h"
#include <memory>
#include <map>
#include <utility>
#include "codegen.h"
#include "lexer.h"
#include "type.h"
#include "symboltable.h"
using namespace AST;
using namespace std;

class Parser
{
    Lexer lexer;
    map<string, unique_ptr<::Type>> GlobalVarTable;
    map<string, unique_ptr<::Type>> LocalVarTable;
    map<string, pair<vector<unique_ptr<::Type>>, unique_ptr<::Type>>> FunctionTable;
    map<string, int> OperatorPrecedence;
    bool parsingFuncDef;
    ::Type *currentFuncType;

public:
    Parser(string Name) : lexer(Lexer(Name)), parsingFuncDef(false)
    {
        OperatorPrecedence["+"] = 10;
        OperatorPrecedence["-"] = 10;
        OperatorPrecedence["*"] = 40;
        OperatorPrecedence["/"] = 40;
        OperatorPrecedence["&"] = 10;
        OperatorPrecedence["|"] = 10;
        OperatorPrecedence["<"] = 10;
        OperatorPrecedence["<="] = 10;
        OperatorPrecedence[">"] = 10;
        OperatorPrecedence[">="] = 10;
        OperatorPrecedence["=="] = 10;
        OperatorPrecedence["!="] = 10;
    }
    void parse();
    unique_ptr<::Type> getType();
    int getOperatorPrecedence();
    unique_ptr<BinOps> returnBinOpsType();
    unique_ptr<Expression> ParseArrayElemExpression(const string &);
    std::unique_ptr<AST::Expression> ParseExpression();
    std::unique_ptr<AST::Expression> ParseParen();
    std::unique_ptr<AST::Statement> ParseStatement();
    std::unique_ptr<AST::Statement> ParseStatementIdentifier();
    std::unique_ptr<CompoundStatement> ParseCompoundStatement();
    std::unique_ptr<Statement> ParseIfElseStatement();
    std::unique_ptr<Statement> ParseForStatement();
    std::unique_ptr<AST::Expression> ParsePrimary();
    std::unique_ptr<FunctionSignature> ParseConsume();
    std::unique_ptr<AST::Expression> ParseCallExpression(const string &);
    std::unique_ptr<AST::Expression> ParseVariable(const string &);
    std::unique_ptr<AST::Expression> ParseIntNum();
    unique_ptr<Expression> ParseBooleanValue();
    std::unique_ptr<AST::Expression> ParseIdentifier();
    std::unique_ptr<AST::Expression> ParseDoubleNum();
    std::unique_ptr<AST::Expression> ParseBinOP(int, unique_ptr<Expression>);
    std::unique_ptr<AST::Statement> ParseVariableDeclarationStatement();
    std::unique_ptr<AST::Statement> ParseArrayVariableDeclarationStatement(const string &, unique_ptr<::Type>);
    std::unique_ptr<AST::Statement> ParseLocalVariableDeclarationStatement();
    unique_ptr<AST::Statement> ParseVariableAssignmentStatement(const string &);
    unique_ptr<AST::VariableAssignment> VariableAssignmentStatementHelper(const string &);
    unique_ptr<AST::Statement> ParseCallStatement(const string &);
    unique_ptr<Expression> ParsePreUnary();
    unique_ptr<Expression> ParsePostUnary(unique_ptr<Expression> e);
    unique_ptr<AST::Statement> ParseReturnStatement();
    std::unique_ptr<FunctionSignature> ParseFunctionSignature();
    std::unique_ptr<FunctionDefinition> ParseFunctionDefinition();
    void LogError(const char *errmsg) { cerr << "Error : " << errmsg << endl; }
    bool isExpression() { return (lexer.isTokenIntNum() || lexer.isTokenDoubleNum() ||
                                  lexer.isTokenIdentifier() || lexer.isTokenLeftParen()) ||
                                 lexer.isTokenTrueValue() ||
                                 lexer.isTokenFalseValue(); }
};