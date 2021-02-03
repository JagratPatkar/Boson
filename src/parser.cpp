#include "parser.h"

void Parser::addVariable(const string& name,Types type){ 
    SymbolTable[name] = type;
 }
bool Parser::doesVariableExist(const string& name){ 
    if(SymbolTable[name]) return true;
    return false;
}
Types Parser::getVariableType(const string& name){
    return SymbolTable[name];
}


void Parser::parse(){
    int token = lexer.getNextToken();
    while(true) 
    {   
        switch(token){
            case -7 : 
                if(ParseVariableAssignmentStatement()){
                    printf("Parsed a Variable Assignment Statement \n");
                }
            break;
            case -4 : 
                if(ParseVariableDeclarationStatement()){
                    printf("Parsed a variable declaration statement of type int \n");
                }
            break;
            case -5 :
                if(ParseVariableDeclarationStatement()){
                    printf("Parsed a variable declaration statement of type double \n");
                }
            break;
            case -1 :printf("End of File \n");
                     exit(1);
                     break;
        }
        token = lexer.getNextToken();
    }
    lexer.closeFile();
}  

unique_ptr<Statement> Parser::ParseVariableDeclarationStatement(){
    int type = lexer.getCurrentToken();
    lexer.getNextToken();
    if(!lexer.isTokenIdentifier()) return LogStatementError("Identifier Expected"); 
    string name = lexer.getIdentifier();
    if(doesVariableExist(name)) return LogStatementError("Illegal Re-declaration");
    Types typedType = AST::TypesOnToken(type);
    addVariable(name,typedType);
    auto var = make_unique<Variable>(name,typedType);
    lexer.getNextToken();
    unique_ptr<Expression> exp = nullptr;
    if(lexer.isTokenAssignmentOp()) {
        lexer.getNextToken();
        exp = ParseExpression();
        int t1 = (int)var->getType();
        int t2 = (int)exp->getType();
        if(t1 != t2) {
            LogTypeError(t1,t2);
            return nullptr;
        }
    }
    lexer.getNextToken();
    if(!lexer.isTokenSemiColon()) return LogStatementError("Expected ';' or '='");
    return make_unique<VariableDeclaration>(move(var),move(exp));
}


unique_ptr<Statement> Parser::ParseVariableAssignmentStatement(){
    string name = lexer.getIdentifier();
    if(!doesVariableExist(name)) return LogStatementError("Undefined Variable");
    int t1 = getVariableType(name);
    auto var = make_unique<Variable>(name,(Types)t1);
    lexer.getNextToken();
    if(!lexer.isTokenAssignmentOp()) return LogStatementError("Missing '=' operator");
    lexer.getNextToken();
    auto exp = ParseExpression();
    int t2 = exp->getType();
    if(t1 != t2){
        LogTypeError(t1,t2);
        return nullptr;
    }
    lexer.getNextToken();
    if(!lexer.isTokenSemiColon()) return LogStatementError("Expected a end of statement");
    return make_unique<VariableAssignment>(move(var),move(exp));
}


unique_ptr<Expression> Parser::LogExpressionError(const char* errmsg){
    fprintf(stderr,"Error : %s\n",errmsg);
    return nullptr;
}

unique_ptr<Statement> Parser::LogStatementError(const char* errmsg){
    fprintf(stderr,"Error : %s\n",errmsg);
    return nullptr;
}

unique_ptr<Expression> Parser::LogTypeError(int t1,int t2){
    fprintf(stderr,"Type mistmatch between : %s - and - %s \n", AST::TypesName(t1) , AST::TypesName(t2));
    return nullptr;
}

unique_ptr<Expression> Parser::ParseExpression(){
    auto lvalue = ParsePrimary();
    if(!lvalue) return nullptr;
    return lvalue;
}

unique_ptr<Expression> Parser::ParsePrimary(){
    if(lexer.isTokenIntNum()) return ParseIntNum();
    if(lexer.isTokenDoubleNum()) return ParseDoubleNum();
    if(lexer.isTokenIdentifier()) return ParseIdentifier();
    else return LogExpressionError("Unknown Expression!");
}   


unique_ptr<Expression> Parser::ParseIdentifier(){
    string Name = lexer.getIdentifier();
    if(doesVariableExist(Name)) return make_unique<Variable>(Name,getVariableType(Name));
    return LogExpressionError("Undefined Variable");
}

unique_ptr<Expression>  Parser::ParseIntNum(){
    return make_unique<IntNum>(lexer.getIntNum());
}

unique_ptr<Expression> Parser::ParseDoubleNum(){
    return make_unique<DoubleNum>(lexer.getDoubleNum());
}