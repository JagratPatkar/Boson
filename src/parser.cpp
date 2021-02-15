#include "parser.h"

extern unique_ptr<Module> module;
extern unique_ptr<IRBuilder<>> builder;
extern BasicBlock *bb;
extern void initialize();
static bool parsingFuncDef = false;
static bool parsingIf_or_For = false;


 BinOps Parser::returnBinOpsType(){
    if(lexer.isTokenAddSym()) return op_add;
    else if(lexer.isTokenSubSym()) return op_sub;
    else if(lexer.isTokenMulSym()) return op_mul;
    else if(lexer.isTokenDivSym()) return op_div;
    else if(lexer.isTokenLessThan()) return op_less_than;
    else if(lexer.isTokenLessThanEq()) return op_less_than_eq;
    else if(lexer.isTokenGreaterThan()) return op_greater_than;
    else if(lexer.isTokenGreaterThanEq()) return op_greater_than_eq;
    else if(lexer.isTokenNotEqualTo()) return op_not_equal_to;
    else if(lexer.isTokenEqualTo()) return op_equal_to;
    return  non_op;
 }

int Parser::getOperatorPrecedence(){
    if(lexer.isTokenAddSym()) return OperatorPrecedence["+"];
    else if(lexer.isTokenSubSym()) return OperatorPrecedence["-"];
    else if(lexer.isTokenMulSym()) return OperatorPrecedence["*"];
    else if(lexer.isTokenDivSym()) return OperatorPrecedence["/"];
    else if(lexer.isTokenLessThan()) return OperatorPrecedence["<"];
    else if(lexer.isTokenLessThanEq()) return OperatorPrecedence["<="];
    else if(lexer.isTokenGreaterThan()) return OperatorPrecedence[">"];
    else if(lexer.isTokenGreaterThanEq()) return OperatorPrecedence[">="];
    else if(lexer.isTokenEqualTo()) return OperatorPrecedence["=="];
     else if(lexer.isTokenNotEqualTo()) return OperatorPrecedence["!="];
    return -1;
}

void Parser::parse(){
    lexer.getNextToken();
    initialize();
    while(true) 
    {   
        if(lexer.isTokenInt() || lexer.isTokenDouble()) ParseVariableDeclarationStatement()->codegen();
        else if(lexer.isTokenFunctionKeyword()) ParseFunctionDefinition()->codeGen();
        else if(lexer.isTokenConsume()) ParseConsume()->codegen();
        else{
            // if(!doesStartExist()) LogError("Start Function Not Found");
            module->dump();
            builder->SetInsertPoint(bb);
            builder->CreateRetVoid();
            lexer.closeFile();
            return;
        }
        lexer.getNextToken();
    }
}  

unique_ptr<Statement> Parser::ParseVariableDeclarationStatement(){
    int type = lexer.getCurrentToken();
    lexer.getNextToken();
    if(!lexer.isTokenIdentifier())  {
        LogError("Identifier Expected"); 
        return nullptr;
    }
    string name = lexer.getIdentifier();
    if(GlobalVarTable.doesElementExist(name)) {
        LogError("Illegal Re-declaration"); 
        return nullptr;
    }
    Types typedType = AST::TypesOnToken(type);
    GlobalVarTable.addElement(name,typedType);
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
    if(!lexer.isTokenSemiColon()){
        LogError("Expected ';' or '='"); 
        return nullptr;
    }
    return make_unique<GlobalVariableDeclaration>(move(var),move(exp));
}

unique_ptr<Statement> Parser::ParseLocalVariableDeclarationStatement(){
    int type = lexer.getCurrentToken();
    lexer.getNextToken();
    if(!lexer.isTokenIdentifier()) {
        LogError("Identifier Expected"); 
        return nullptr;
    }
    string name = lexer.getIdentifier();
    if(LocalVarTable.doesElementExist(name)) {
        LogError("Illegal Re-declaration"); 
        return nullptr;
    }
    Types typedType = AST::TypesOnToken(type);
    LocalVarTable.addElement(name,typedType);
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
    if(!lexer.isTokenSemiColon()) {
        LogError("Expected ';' or '='"); 
        return nullptr;
    }
    return make_unique<LocalVariableDeclaration>(move(var),move(exp));
}

unique_ptr<Statement> Parser::ParseVariableAssignmentStatement(const string& name){
    int t1 ;
    if(LocalVarTable.doesElementExist(name)) t1 = LocalVarTable.getElement(name);
    else if(GlobalVarTable.doesElementExist(name)) t1 = GlobalVarTable.getElement(name);
    else {
        LogError("Undefined Variable"); 
        return nullptr;
    }
    auto var = make_unique<Variable>(name,(Types)t1);
    if(!lexer.isTokenAssignmentOp()) {
        LogError("Missing '=' operator"); 
        return nullptr;
    }
    lexer.getNextToken();
    auto exp = ParseExpression();
    int t2 = exp->getType();
    if(t1 != t2){
        LogTypeError(t1,t2);
        return nullptr;
    }
    if(!lexer.isTokenSemiColon()) {
        LogError("Expected a end of statement"); 
        return nullptr;
    }
    return make_unique<VariableAssignment>(move(var),move(exp));
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
    if(!lexer.isTokenRightParen())  {
        LogError("Expression might be missing ')'"); 
        return nullptr;
    }
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
    else {
        LogError("Unknown Expression!"); 
        return nullptr;
    }
}   


unique_ptr<Expression> Parser::ParseVariable(const string& Name){
    if(parsingFuncDef && LocalVarTable.doesElementExist(Name))
        return make_unique<Variable>(Name,LocalVarTable.getElement(Name)); 
    if(GlobalVarTable.doesElementExist(Name)) return make_unique<Variable>(Name,GlobalVarTable.getElement(Name));
    LogError("Undefined Variable"); 
    return nullptr;
}

bool Parser::isExpression(){
    return (lexer.isTokenIntNum() || lexer.isTokenDoubleNum() || 
    lexer.isTokenIdentifier() || lexer.isTokenLeftParen());
}

unique_ptr<Expression> Parser::ParseCallExpression(const string& Name){
    if(!FunctionTable.doesElementExist(Name)) {
        LogError("Undefined Function!"); 
        return nullptr;
    }
    lexer.getNextToken();
    vector<unique_ptr<Expression>> Args;
    vector<Types> ArgType;
    while(isExpression()){
        auto Exp = ParseExpression();
        if(!lexer.isTokenComma() && !lexer.isTokenRightParen()) {
            LogError("Expression might be missing a ','  or a ')'"); 
            return nullptr;
        }
        ArgType.push_back(Exp.get()->getType());
        Args.push_back(move(Exp));
        if(!lexer.isTokenRightParen())lexer.getNextToken();
        else break;
    } 
    pair<vector<Types>,Types> p = FunctionTable.getElement(Name);
    if(ArgType.size() != p.first.size()){
            LogError("Illegal Number of Arguments"); 
            return nullptr;
    }
    for(auto a = ArgType.begin(), b = p.first.begin(); a!=ArgType.end(); a++,b++){
        if(*a != *b){
            return LogTypeError(*a,*b);
        }
    }
    lexer.getNextToken();
    return make_unique<FunctionCall>(Name,move(Args),move(p.second));
}   

unique_ptr<Expression> Parser::ParseIdentifier(){
    string Name = lexer.getIdentifier();
    lexer.getNextToken();
    if(lexer.isTokenLeftParen()) return ParseCallExpression(Name);
    else return ParseVariable(Name);
}

unique_ptr<Expression>  Parser::ParseIntNum(){
    lexer.getNextToken();
    return make_unique<IntNum>(lexer.getIntNum());
}

unique_ptr<Expression> Parser::ParseDoubleNum(){
    lexer.getNextToken();
    return make_unique<DoubleNum>(lexer.getDoubleNum());
}


unique_ptr<Statement> Parser::ParseCallStatement(const string& name){
    if(!FunctionTable.doesElementExist(name)) {
        LogError("Undefined Function!"); 
        return nullptr;
    }
    vector<unique_ptr<Expression>> Args;
    vector<Types> ArgType;
    lexer.getNextToken();
    while(isExpression()){
        auto Exp = ParseExpression();
        if(!lexer.isTokenComma() && !lexer.isTokenRightParen()) {
            LogError("Statement might be missing a ','  or a ')'"); 
            return nullptr;
        }
        ArgType.push_back(Exp.get()->getType());
        Args.push_back(move(Exp));
        if(!lexer.isTokenRightParen())lexer.getNextToken();
        else break;
    } 
    lexer.getNextToken();
    if(!lexer.isTokenSemiColon()){
        LogError("Expected a ';' at the end of statement"); 
        return nullptr;
    }
    pair<vector<Types>,Types> p = FunctionTable.getElement(name);
    if(ArgType.size() != p.first.size()) {
        LogError("Illegal Number of Arguments"); 
        return nullptr;
    }
    for(auto a = ArgType.begin(), b = p.first.begin(); a!=ArgType.end(); a++,b++){
        if(*a != *b){
            LogTypeError(*a,*b);
            LogError("Type mismatch in statement"); 
            return nullptr;
        }
    }
    return make_unique<FunctionCall>(name,move(Args),p.second);
}

unique_ptr<Statement> Parser::ParseStatementIdentifier(){
    string Name = lexer.getIdentifier();
    lexer.getNextToken();
    if(lexer.isTokenLeftParen()) return ParseCallStatement(Name);
    else return ParseVariableAssignmentStatement(Name);
}

unique_ptr<Statement> Parser::ParseStatement(){
    if(lexer.isTokenIdentifier()) return ParseStatementIdentifier();
    if(lexer.isTokenInt()) return ParseLocalVariableDeclarationStatement();
    if(lexer.isTokenDouble()) return ParseLocalVariableDeclarationStatement();
    if(lexer.isTokenReturnKeyword()) return ParseReturnStatement();
    if(lexer.isTokenIf()) return ParseIfElseStatement();
    if(lexer.isTokenFor()) return ParseForStatement();
    LogError("Unknown statement!");
    return nullptr;
}

unique_ptr<Statement> Parser::ParseReturnStatement(){
    if(parsingIf_or_For) {
        LogError("Illeagal use of 'return' statement"); 
        return nullptr;
    }
    lexer.getNextToken();
    if(lexer.isTokenSemiColon()) return make_unique<Return>(nullptr);
    auto exp = ParseExpression();
    if(!exp) return nullptr;
    return make_unique<Return>(move(exp));
}


unique_ptr<Statement> Parser::ParseIfElseStatement(){
    parsingIf_or_For = true;
    lexer.getNextToken();
    if(!lexer.isTokenLeftParen()) {
        LogError("Expected a '(' befor expression"); 
        return nullptr;
    }
    lexer.getNextToken();
    auto Exp = ParseExpression();
    if(!lexer.isTokenRightParen()){
        LogError("Expected a ')' after expression"); 
        return nullptr;
    }
    lexer.getNextToken();
    auto CmpStat = ParseCompoundStatement();
    lexer.getNextToken();
    if(!lexer.isTokenElse()) {
        LogError("Expected a 'else' after 'if'"); 
        return nullptr;
    }
    lexer.getNextToken();
    auto elseCmpStat = ParseCompoundStatement();
    parsingIf_or_For = false;
    return make_unique<IfElseStatement>(move(Exp),move(CmpStat),move(elseCmpStat));
}



unique_ptr<VariableAssignment> Parser::VariableAssignmentStatementHelper(const string& name){
    int t1;
    if(LocalVarTable.doesElementExist(name)) t1 = LocalVarTable.getElement(name);
    else if(GlobalVarTable.doesElementExist(name)) t1 = GlobalVarTable.getElement(name);
    else {
        LogError("Undefined Variable");
        return nullptr;
    } 
    auto var = make_unique<Variable>(name,(Types)t1);
    if(!lexer.isTokenAssignmentOp()) {
        LogError("Missing '=' operator");
        return nullptr;
    } 
    lexer.getNextToken();
    auto exp = ParseExpression();
    int t2 = exp->getType();
    if(t1 != t2){
        LogTypeError(t1,t2);
        return nullptr;
    }
    return make_unique<VariableAssignment>(move(var),move(exp));
}

unique_ptr<Statement> Parser::ParseForStatement(){
    parsingIf_or_For = true;
    unique_ptr<LocalVariableDeclaration> lvd;
    unique_ptr<VariableAssignment> va;
    unique_ptr<VariableAssignment> vastep;
    lexer.getNextToken();
    if(!lexer.isTokenLeftParen()) {
        LogError("Expected a '(' in 'for'");
        return nullptr;
    } 
    lexer.getNextToken();
    if(lexer.isTokenInt() || lexer.isTokenDouble()) {
        int type = lexer.getCurrentToken();
        lexer.getNextToken();
        if(!lexer.isTokenIdentifier()) {
            LogError("Identifier Expected");
            return nullptr;
        }  
        string name = lexer.getIdentifier();
        if(LocalVarTable.doesElementExist(name)) {
            LogError("Illegal Re-declaration");
            return nullptr;
        } 
        Types typedType = AST::TypesOnToken(type);
        LocalVarTable.addElement(name,typedType);
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
        if(!lexer.isTokenSemiColon()){
            LogError("Expected ';' or '='");
            return nullptr;
        }  
        lvd =  make_unique<LocalVariableDeclaration>(move(var),move(exp));
    }
    else if(lexer.isTokenIdentifier()) {
        string Name = lexer.getIdentifier();
        lexer.getNextToken();
        va = VariableAssignmentStatementHelper(Name);
        if(!lexer.isTokenSemiColon()){
            LogError("Expected a ';' in 'for'");
            return nullptr;
        } 
        lvd = nullptr;
    }
    else if(lexer.isTokenSemiColon()) {
        lvd = nullptr;
        va = nullptr;
    }
    else{
        LogError("Unknown Statement in 'for'");
        return nullptr;
    }

    lexer.getNextToken();
    auto cond = ParseExpression();
    if(!cond){
        LogError("Illegal condition in 'for'");
        return nullptr;
    } 
    if(!lexer.isTokenSemiColon()) {
        LogError("Expected a ';' in 'for'");
        return nullptr;
    } 

    lexer.getNextToken();
    if(lexer.isTokenIdentifier()){
        string Name = lexer.getIdentifier();
        lexer.getNextToken();
        vastep = VariableAssignmentStatementHelper(Name);
        if(!lexer.isTokenRightParen()) {
            LogError("Expected a ')' in 'for'");
            return nullptr;
        } 
    } 
    else if(!lexer.isTokenRightParen()) {
            LogError("Expected a ')' in 'for'");
            return nullptr;
    }
    lexer.getNextToken();
    auto cmpStat = ParseCompoundStatement();
    parsingIf_or_For = false;
    return make_unique<ForStatement>(move(lvd),move(va),move(cond),move(vastep),move(cmpStat));
}

unique_ptr<FunctionSignature> Parser::ParseConsume(){
    lexer.getNextToken();
    if(!lexer.isTokenFunctionKeyword()) {
            LogError("Expected keyword 'fn'");
            return nullptr;
    }
    lexer.getNextToken();
    auto fs =  ParseFunctionSignature();
    lexer.getNextToken();
    if(!lexer.isTokenSemiColon()) {
            LogError("Expected a ';' after 'consume'");
            return nullptr;
    }
    return fs;
}

unique_ptr<FunctionSignature> Parser::ParseFunctionSignature(){
    if(!lexer.isTokenIdentifier()) {
            LogError("Expected an Identifier");
            return nullptr;
    }
    string Name = lexer.getIdentifier();
    if(FunctionTable.doesElementExist(Name)){
            LogError("Function Already Defined");
            return nullptr;
    }
    vector<Types> argType;
    lexer.getNextToken();
    if(!lexer.isTokenLeftParen()){
            LogError("Missing '(' in declaration");
            return nullptr;
    } 
    lexer.getNextToken();
    vector<unique_ptr<Variable>> args;
    while(lexer.isTokenInt() || lexer.isTokenDouble()){
        int type = lexer.getCurrentToken();
        lexer.getNextToken();
        if(!lexer.isTokenIdentifier()) {
            LogError("Identifier Expected");
            return nullptr;
        } 
        string name = lexer.getIdentifier();
        if(LocalVarTable.doesElementExist(name)) {
            LogError("Illegal Re-declaration");
            return nullptr;
        } 
        Types typedType = AST::TypesOnToken(type);
        argType.push_back(typedType);
        LocalVarTable.addElement(name,typedType);
        auto var = make_unique<Variable>(name,typedType);
        args.push_back(move(var));
        lexer.getNextToken();
        if(lexer.isTokenRightParen()) break;
        if(!lexer.isTokenComma()) {
            LogError("Expected a ',' between arguments");
            return nullptr;
        }
        lexer.getNextToken(); 
    }
    if(!lexer.isTokenRightParen()){
        LogError("Missing ')' in declaration");
        return nullptr;
    } 
    lexer.getNextToken();
    if(!lexer.isAnyType()) {
        LogError("Incomplete Type Specification in Function Declaration");
        return nullptr;
    }
    Types type = AST::TypesOnToken(lexer.getCurrentToken());
    Types typecopy2 = AST::TypesOnToken(lexer.getCurrentToken());
    FunctionTable.addElement(Name,make_pair(move(argType),move(typecopy2)));
    return make_unique<FunctionSignature>(move(Name),move(type),move(args)); 
}

unique_ptr<CompoundStatement> Parser::ParseCompoundStatement(){
    if(!lexer.isTokenLeftCurlyBrace()) {
        LogError("Missing '{' in Statement");
        return nullptr;
    }
    lexer.getNextToken();
    vector<unique_ptr<Statement>> statements;
    while(!lexer.isTokenRightCurlyBrace()){
        auto stat = ParseStatement();
        if(!stat) {
            LogError("Statement Might be missing '}'");
            return nullptr;
        }
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
    if(!CS->isLastElementReturnStatement())  {
        LogError("Missing 'return' statement at the end of Function Definition");
        return nullptr;
    }
    int t1 = CS->returnReturnStatementType();
    int t2 = FH->getRetType();  
    if(t1 != t2) {
        LogTypeError(t1,t2);
        LogError("Type mismatch in 'return' statement and return 'type'");
        return nullptr;
    } 
    parsingFuncDef = false;
    LocalVarTable.clearTable();
    return make_unique<FunctionDefinition>(move(FH),move(CS));
}