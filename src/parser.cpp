#include "parser.h"

extern unique_ptr<Module> module ;
extern void initialize();
static bool parsingFuncDef = false;
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

void Parser::addVariableLocally(const string& name,Types type){ 
    SymbolTableLocal[name] = type;
 }
bool Parser::doesVariableExistLocally(const string& name){ 
    if(SymbolTableLocal[name]) return true;
    return false;
}
Types Parser::getVariableTypeLocally(const string& name){
    return SymbolTableLocal[name];
}

 void Parser::clearVariablesLocally(){
     SymbolTableLocal.clear();
 }

 BinOps Parser::returnBinOpsType(){
    if(lexer.isTokenAddSym()) return op_add;
    else if(lexer.isTokenSubSym()) return op_sub;
    else if(lexer.isTokenMulSym()) return op_mul;
    else if(lexer.isTokenDivSym()) return op_div;
    return  non_op;
 }

int Parser::getOperatorPrecedence(){
    if(lexer.isTokenAddSym()) return OperatorPrecedence['+'];
    else if(lexer.isTokenSubSym()) return OperatorPrecedence['-'];
    else if(lexer.isTokenMulSym()) return OperatorPrecedence['*'];
    else if(lexer.isTokenDivSym()) return OperatorPrecedence['/'];
    return -1;
}

void Parser::parse(){
    int token = lexer.getNextToken();
    initialize();
    while(true) 
    {   
        switch(token){
            // case -7 : 
            //     if(ParseVariableAssignmentStatement()){
            //         printf("Parsed a Variable Assignment Statement \n");
            //     }
            // break;
            case -4 : 
                if(auto VD = ParseVariableDeclarationStatement()){
                    printf("Parsed a variable declaration statement of type int \n");
                    VD->codeGen();
                }
            break;
            case -5 :
                if(auto VD = ParseVariableDeclarationStatement()){
                    printf("Parsed a variable declaration statement of type double \n");
                    VD->codeGen();
                }
            break;
            case -16 :
                if(auto FD = ParseFunctionDefinition()){
                    printf("Parsed a function definition \n");
                    FD->codeGen();
                }
            break;
            case -1 :printf("End of File \n");
                     module->dump();
                     lexer.closeFile();
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
    if(!lexer.isTokenSemiColon()) return LogStatementError("Expected ';' or '='");
    return make_unique<GlobalVariableDeclaration>(move(var),move(exp));
}

unique_ptr<Statement> Parser::ParseLocalVariableDeclarationStatement(){
    int type = lexer.getCurrentToken();
    lexer.getNextToken();
    if(!lexer.isTokenIdentifier()) return LogStatementError("Identifier Expected"); 
    string name = lexer.getIdentifier();
    if(doesVariableExistLocally(name)) return LogStatementError("Illegal Re-declaration");
    Types typedType = AST::TypesOnToken(type);
    addVariableLocally(name,typedType);
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
    if(!lexer.isTokenSemiColon()) return LogStatementError("Expected ';' or '='");
    return make_unique<LocalVariableDeclaration>(move(var),move(exp));
}

unique_ptr<Statement> Parser::ParseVariableAssignmentStatement(){
    string name = lexer.getIdentifier();
    int t1 ;
    if(doesVariableExistLocally(name)) t1 = getVariableTypeLocally(name);
    else if(doesVariableExist(name)) t1 = getVariableType(name);
    else return LogStatementError("Undefined Variable");
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


unique_ptr<CompoundStatement> Parser::LogCompStatementError(const char* errmsg){
    fprintf(stderr,"Error : %s\n",errmsg);
    return nullptr;
}

unique_ptr<FunctionSignature> Parser::LogFuncSigError(const char* errmsg){
    fprintf(stderr,"Error : %s\n",errmsg);
    return nullptr;
}


unique_ptr<FunctionDefinition> Parser::LogFuncDefError(const char* errmsg){
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
    return ParseBinOP(0,move(lvalue));
}

unique_ptr<Expression> Parser::ParseParen(){
    lexer.getNextToken();
    auto exp = ParseExpression();
    if(!lexer.isTokenRightParen())  LogExpressionError("Expression might be missing ')'");
    lexer.getNextToken();
    return move(exp);
}


unique_ptr<Expression> Parser::ParseBinOP(int minPrec,unique_ptr<Expression> lvalue){
    while(true)
    {
        int prevPrec = getOperatorPrecedence();
        if(prevPrec < minPrec) return move(lvalue);
        BinOps op_typ = returnBinOpsType();
        lexer.getNextToken();
        auto rvalue = ParsePrimary();
        if(!rvalue) return nullptr;
        Types t = lvalue->getType();
        int t1 = (int)t;
        int t2 = rvalue->getType();
        if(t1 != t2) return LogTypeError(t1,t2);
        if(prevPrec < getOperatorPrecedence()){
            rvalue = ParseBinOP(prevPrec+1,move(rvalue));
            if(!rvalue) return nullptr;
        }
        lvalue = make_unique<BinaryExpression>(op_typ,move(lvalue),move(rvalue),t);
    }
}   

unique_ptr<Expression> Parser::ParsePrimary(){
    if(lexer.isTokenIntNum()) return ParseIntNum();
    if(lexer.isTokenDoubleNum()) return ParseDoubleNum();
    if(lexer.isTokenIdentifier()) return ParseIdentifier();
    if(lexer.isTokenLeftParen()) return ParseParen();
    else return LogExpressionError("Unknown Expression!");
}   


unique_ptr<Expression> Parser::ParseIdentifier(){
    string Name = lexer.getIdentifier();
    if(parsingFuncDef){
        if(doesVariableExistLocally(Name)){
            lexer.getNextToken();
            return make_unique<Variable>(Name,getVariableTypeLocally(Name));
        }
    }
    if(doesVariableExist(Name)) {
        lexer.getNextToken();
        return make_unique<Variable>(Name,getVariableType(Name));
    }
    return LogExpressionError("Undefined Variable");
}

unique_ptr<Expression>  Parser::ParseIntNum(){
    lexer.getNextToken();
    return make_unique<IntNum>(lexer.getIntNum());
}

unique_ptr<Expression> Parser::ParseDoubleNum(){
    lexer.getNextToken();
    return make_unique<DoubleNum>(lexer.getDoubleNum());
}


unique_ptr<Statement> Parser::ParseStatement(){
    if(lexer.isTokenIdentifier()) return ParseVariableAssignmentStatement();
    if(lexer.isTokenInt()) return ParseLocalVariableDeclarationStatement();
    if(lexer.isTokenDouble()) return ParseLocalVariableDeclarationStatement();
    if(lexer.isTokenReturnKeyword()) return ParseReturnStatement();
    return LogStatementError("Unknown statement!");
}

unique_ptr<Statement> Parser::ParseReturnStatement(){
    lexer.getNextToken();
    if(lexer.isTokenSemiColon()) return make_unique<Return>(nullptr);
    auto exp = ParseExpression();
    if(!exp) return nullptr;
    return make_unique<Return>(move(exp));
}

unique_ptr<FunctionSignature> Parser::ParseFunctionSignature(){
    if(!lexer.isTokenIdentifier()) return LogFuncSigError("Expected an Identifier");
    string Name = lexer.getIdentifier();
    lexer.getNextToken();
    if(!lexer.isTokenLeftParen()) return LogFuncSigError("Missing '(' in declaration");

    //Parse Parameters Later here

    lexer.getNextToken();
    if(!lexer.isTokenRightParen()) return LogFuncSigError("Missing ')' in declaration");

    lexer.getNextToken();
    if(!lexer.isAnyType()) return LogFuncSigError("Incomplete Type Specification in Function Declaration"); 
    Types type = AST::TypesOnToken(lexer.getCurrentToken());

    return make_unique<FunctionSignature>(move(Name),move(type)); 
}

unique_ptr<CompoundStatement> Parser::ParseCompoundStatement(){
    if(!lexer.isTokenLeftCurlyBrace()) return LogCompStatementError("Missing '{' in Statement");
    lexer.getNextToken();
    vector<unique_ptr<Statement>> statements;
    while(!lexer.isTokenRightCurlyBrace()){
        auto stat = ParseStatement();
        if(!stat) return LogCompStatementError("Statement Might be missing '}'");
        statements.push_back(move(stat));
        lexer.getNextToken();
    }
    return make_unique<CompoundStatement>(move(statements));
}

unique_ptr<FunctionDefinition> Parser::ParseFunctionDefinition(){
    lexer.getNextToken();
    auto FH = ParseFunctionSignature();
    if(!FH) return nullptr;
    lexer.getNextToken();
    parsingFuncDef = true;
    auto CS = ParseCompoundStatement();
    if(!CS) return nullptr;
    if(!CS->isLastElementReturnStatement()) 
        return LogFuncDefError("Missing 'return' statement at the end of Function Definition");
    int t1 = CS->returnReturnStatementType();
    int t2 = FH->getRetType();  
    if(t1 != t2) {
        LogTypeError(t1,t2);
        return LogFuncDefError("Type mismatch in 'return' statement and return 'type'");
    } 
    parsingFuncDef = false;
    clearVariablesLocally();
    return make_unique<FunctionDefinition>(move(FH),move(CS));
}