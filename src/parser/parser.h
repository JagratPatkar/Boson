#include "../ast/ast.h"
#include <memory>
#include <map>
#include <utility>
#include "../codegen/codegen.h"
#include "../lexer/lexer.h"
using namespace AST;
using namespace std;

class Parser
{
    Lexer lexer;
    AST::SymbolTable<string, Types> GlobalVarTable;
    AST::SymbolTable<string, Types> LocalVarTable;
    AST::SymbolTable<string, pair<vector<Types>, Types>> FunctionTable;
    map<string, int> OperatorPrecedence;
    bool parsingFuncDef;
    bool parsingIf_or_For;

public:
    Parser(string Name) : lexer(Lexer(Name)), parsingFuncDef(false), parsingIf_or_For(false)
    {
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
        GlobalVarTable = AST::SymbolTable<string, Types>();
        LocalVarTable = AST::SymbolTable<string, Types>();
        FunctionTable = AST::SymbolTable<string, pair<vector<Types>, Types>>();
    }
    void parse();
    int getOperatorPrecedence();
    BinOps returnBinOpsType();
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
    std::unique_ptr<AST::Expression> ParseIdentifier();
    std::unique_ptr<AST::Expression> ParseDoubleNum();
    std::unique_ptr<AST::Expression> ParseBinOP(int, unique_ptr<Expression>);
    std::unique_ptr<AST::Statement> ParseVariableDeclarationStatement();
    std::unique_ptr<AST::Statement> ParseLocalVariableDeclarationStatement();
    unique_ptr<AST::Statement> ParseVariableAssignmentStatement(const string &);
    unique_ptr<AST::VariableAssignment> VariableAssignmentStatementHelper(const string &);
    unique_ptr<AST::Statement> ParseCallStatement(const string &);
    unique_ptr<AST::Statement> ParseReturnStatement();
    std::unique_ptr<FunctionSignature> ParseFunctionSignature();
    std::unique_ptr<FunctionDefinition> ParseFunctionDefinition();
    void LogError(const char *errmsg) { fprintf(stderr, "Error : %s\n", errmsg); }
    std::unique_ptr<AST::Expression> LogTypeError(int, int);
    bool isExpression() { return (lexer.isTokenIntNum() || lexer.isTokenDoubleNum() ||
                                  lexer.isTokenIdentifier() || lexer.isTokenLeftParen()); }
};