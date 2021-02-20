#include "parser.h"

#include "llvm/IR/Verifier.h"

static CodeGen* cg = CodeGen::GetInstance();

void Parser::parse()
{
    lexer.getNextToken();
    while (true)
    {
        if (lexer.isTokenInt() || lexer.isTokenDouble() || lexer.isTokenBoolean())
        {  
            ParseVariableDeclarationStatement()->codegen();
        }
        else if (lexer.isTokenFunctionKeyword())
            ParseFunctionDefinition()->codeGen();
        else if (lexer.isTokenConsume())
            ParseConsume()->codegen();
        else
        {
            if (FunctionTable.find("start") == FunctionTable.end())
                LogError("Start Function Not Found");
            cg->terminateCOPBB();
            cg->dumpIR();
            lexer.closeFile();
            return;
        }
        lexer.getNextToken();
    }
}


unique_ptr<Expression> Parser::LogTypeError(int t1, int t2)
{
    // fprintf(stderr, "Type mistmatch between : %s - and - %s \n", AST::TypesName(t1), AST::TypesName(t2));
    return nullptr;
}

unique_ptr<Expression> Parser::ParseExpression()
{
    auto lvalue = ParsePrimary();
    if (!lvalue)
        return nullptr;
    return ParseBinOP(0, move(lvalue));
}

unique_ptr<Expression> Parser::ParsePrimary()
{
    if (lexer.isTokenIntNum())
        return ParseIntNum();
    if (lexer.isTokenDoubleNum())
        return ParseDoubleNum();
    if (lexer.isTokenTrueValue() || lexer.isTokenFalseValue() )
        return ParseBooleanValue();
    if (lexer.isTokenIdentifier())
        return ParseIdentifier();
    if (lexer.isTokenLeftParen())
        return ParseParen();
    else
    {
        LogError("Unknown Expression!");
        return nullptr;
    }
}

unique_ptr<Expression> Parser::ParseVariable(const string &Name)
{
    if (parsingFuncDef && LocalVarTable[Name])
        return make_unique<Variable>(Name, LocalVarTable[Name].get()->getNew());
    if (GlobalVarTable[Name])
        return make_unique<Variable>(Name, GlobalVarTable[Name].get()->getNew());
    LogError("Undefined Variable");
    return nullptr;
}


unique_ptr<Expression> Parser::ParseIdentifier()
{
    string Name = lexer.getIdentifier();
    lexer.getNextToken();
    if (lexer.isTokenLeftParen())
        return ParseCallExpression(Name);
    else
        return ParseVariable(Name);
}

unique_ptr<Expression> Parser::ParseIntNum()
{
    lexer.getNextToken();
    return make_unique<IntNum>(lexer.getIntNum());
}

unique_ptr<Expression> Parser::ParseDoubleNum()
{
    lexer.getNextToken();
    return make_unique<DoubleNum>(lexer.getDoubleNum());
}

unique_ptr<Expression> Parser::ParseBooleanValue(){
    if(lexer.isTokenTrueValue()){
        lexer.getNextToken();
        return make_unique<Boolean>(true);
    }else if(lexer.isTokenFalseValue()){
        lexer.getNextToken();
        return make_unique<Boolean>(false);
    }
    return nullptr;
}

unique_ptr<Expression> Parser::ParseParen()
{
    lexer.getNextToken();
    auto exp = ParseExpression();
    if (!lexer.isTokenRightParen())
    {
        LogError("Expression might be missing ')'");
        return nullptr;
    }
    lexer.getNextToken();
    return move(exp);
}

unique_ptr<::Type> Parser::getType(){
        if(lexer.isTokenInt()) return make_unique<Int>();
        if(lexer.isTokenDouble()) return make_unique<Double>();
        if(lexer.isTokenVoid()) return make_unique<Void>();
        if(lexer.isTokenBoolean()) return make_unique<Bool>();
        return nullptr;
}

unique_ptr<BinOps> Parser::returnBinOpsType()
{
    if (lexer.isTokenAddSym())
        return make_unique<OpAdd>();
    else if (lexer.isTokenSubSym())
        return make_unique<OpSub>();
    else if (lexer.isTokenMulSym())
        return make_unique<OpMul>();
    else if (lexer.isTokenDivSym())
        return make_unique<OpDiv>();
    else if (lexer.isTokenLessThan())
        return make_unique<OpLessThan>();
    else if (lexer.isTokenLessThanEq())
        return make_unique<OpLessThanEq>();
    else if (lexer.isTokenGreaterThan())
        return make_unique<OpGreaterThan>();
    else if (lexer.isTokenGreaterThanEq())
        return make_unique<OpGreaterThanEq>();
    else if (lexer.isTokenNotEqualTo())
        return make_unique<OpNotEqualTo>();
    else if (lexer.isTokenEqualTo())
        return make_unique<OpEqualTo>();
    else if (lexer.isTokenAmpersand())
        return make_unique<OpAnd>();
    else if (lexer.isTokenVerticalLine())
        return make_unique<OpOr>();
    LogError("Udefined Operator");
    return NULL;
}

int Parser::getOperatorPrecedence()
{
    if (lexer.isTokenAddSym())
        return OperatorPrecedence["+"];
    else if (lexer.isTokenSubSym())
        return OperatorPrecedence["-"];
    else if (lexer.isTokenMulSym())
        return OperatorPrecedence["*"];
    else if (lexer.isTokenDivSym())
        return OperatorPrecedence["/"];
    else if (lexer.isTokenLessThan())
        return OperatorPrecedence["<"];
    else if (lexer.isTokenLessThanEq())
        return OperatorPrecedence["<="];
    else if (lexer.isTokenGreaterThan())
        return OperatorPrecedence[">"];
    else if (lexer.isTokenGreaterThanEq())
        return OperatorPrecedence[">="];
    else if (lexer.isTokenEqualTo())
        return OperatorPrecedence["=="];
    else if (lexer.isTokenNotEqualTo())
        return OperatorPrecedence["!="];
    else if (lexer.isTokenAmpersand())
        return OperatorPrecedence["&"];
    else if (lexer.isTokenVerticalLine())
        return OperatorPrecedence["|"];
    return -1;
}

unique_ptr<Expression> Parser::ParseBinOP(int minPrec, unique_ptr<Expression> lvalue)
{
    while (true)
    {
        int prevPrec = getOperatorPrecedence();
        if (prevPrec < minPrec)
            return move(lvalue);
        auto BinOp  = returnBinOpsType();
        lexer.getNextToken();
        auto rvalue = ParsePrimary();
        if (!rvalue)
            return nullptr;
        
        auto ltype = lvalue->getType();
        auto rtype = rvalue->getType();
        if (!ltype->doesMatch(rtype))
            LogError("Type Error");
        if(!BinOp->validOperandSet(ltype)){
            LogError("Type of Operands given to Operator is Illeagal");
        }
        if (prevPrec < getOperatorPrecedence())
        {
            rvalue = ParseBinOP(prevPrec + 1, move(rvalue));
            if (!rvalue)
                return nullptr;
        }
        auto OpType = BinOp->getOperatorEvalTy();
        lvalue = make_unique<BinaryExpression>(move(BinOp), move(lvalue), move(rvalue), move(OpType));
    }
}

unique_ptr<Expression> Parser::ParseCallExpression(const string &Name)
{
    if (FunctionTable.find(Name) == FunctionTable.end())
    {
        LogError("Undefined Function!");
        return nullptr;
    }
    lexer.getNextToken();
    vector<unique_ptr<Expression>> Args;
    vector<unique_ptr<::Type>> ArgType;
    while (isExpression())
    {
        auto Exp = ParseExpression();
        if (!lexer.isTokenComma() && !lexer.isTokenRightParen())
        {
            LogError("Expression might be missing a ','  or a ')'");
            return nullptr;
        }
        ArgType.push_back(Exp->getType()->getNew());
        Args.push_back(move(Exp));
        if (!lexer.isTokenRightParen())
            lexer.getNextToken();
        else
            break;
    }
    pair<vector<unique_ptr<::Type>>, unique_ptr<::Type>> p = move(FunctionTable[Name]);
    if (ArgType.size() != p.first.size())
    {
        LogError("Illegal Number of Arguments");
        return nullptr;
    }
    for (auto a = ArgType.begin(), b = p.first.begin(); a != ArgType.end(); a++, b++)
    {
        if(!a->get()->doesMatch(b->get()))
        {
             LogError("Type Mismatch");
        }
    }
    auto sec = p.second.get()->getNew();
    FunctionTable[Name] = move(p);
    lexer.getNextToken();
    return make_unique<FunctionCall>(Name, move(Args), move(sec));
}

unique_ptr<Statement> Parser::ParseStatement()
{
    if (lexer.isTokenIdentifier())
        return ParseStatementIdentifier();
    if (lexer.isTokenInt() || lexer.isTokenDouble() || lexer.isTokenBoolean())
        return ParseVariableDeclarationStatement();
    if (lexer.isTokenReturnKeyword())
        return ParseReturnStatement();
    if (lexer.isTokenIf())
        return ParseIfElseStatement();
    if (lexer.isTokenFor())
        return ParseForStatement();
    LogError("Unknown statement!");
    return nullptr;
}

unique_ptr<Statement> Parser::ParseStatementIdentifier()
{
    string Name = lexer.getIdentifier();
    lexer.getNextToken();
    if (lexer.isTokenLeftParen())
        return ParseCallStatement(Name);
    else
        return ParseVariableAssignmentStatement(Name);
}



unique_ptr<Statement> Parser::ParseVariableAssignmentStatement(const string &name)
{
    auto VA = VariableAssignmentStatementHelper(name);
    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected a end of statement");
        return nullptr;
    }
    return move(VA);
}


unique_ptr<Statement> Parser::ParseCallStatement(const string &name)
{
    if (FunctionTable.find(name) == FunctionTable.end())
    {
        LogError("Undefined Function!");
        return nullptr;
    }
    vector<unique_ptr<Expression>> Args;
    vector<unique_ptr<::Type>> ArgType;
    lexer.getNextToken();
    while (isExpression())
    {
        auto Exp = ParseExpression();
        if (!lexer.isTokenComma() && !lexer.isTokenRightParen())
        {
            LogError("Statement might be missing a ','  or a ')'");
            return nullptr;
        }
        ArgType.push_back(Exp->getType()->getNew());
        Args.push_back(move(Exp));
        if (!lexer.isTokenRightParen())
            lexer.getNextToken();
        else
            break;
    }
    lexer.getNextToken();
    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected a ';' at the end of statement");
        return nullptr;
    }
    pair<vector<unique_ptr<::Type>>, unique_ptr<::Type>> p = move(FunctionTable[name]);
    if (ArgType.size() != p.first.size())
    {
        LogError("Illegal Number of Arguments");
        return nullptr;
    }
    for (auto a = ArgType.begin(), b = p.first.begin(); a != ArgType.end(); a++, b++)
    {
        if(!a->get()->doesMatch(b->get()))
        {
             LogError("Type Mismatch");
        }
    }
    auto sec = p.second.get()->getNew();
    FunctionTable[name] = move(p);
    return make_unique<FunctionCall>(name, move(Args), move(p.second));
}



unique_ptr<Statement> Parser::ParseVariableDeclarationStatement()
{
    unique_ptr<::Type> ty = getType();
    lexer.getNextToken();
    if (!lexer.isTokenIdentifier())
    {
        LogError("Identifier Expected");
        return nullptr;
    }
    string name = lexer.getIdentifier();
    if(parsingFuncDef){
        if (LocalVarTable[name])
        {
            LogError("Illegal Re-declaration");
            return nullptr;
        }
    }else{
        if (GlobalVarTable[name])
        {
            LogError("Illegal Re-declaration");
            return nullptr;
        }
    }
    if(!parsingFuncDef)GlobalVarTable[name] = ty->getNew();
    else LocalVarTable[name] = ty->getNew();
    auto var = make_unique<Variable>(name, move(ty->getNew()));
    lexer.getNextToken();
    unique_ptr<Expression> exp = nullptr;
    if (lexer.isTokenAssignmentOp())
    {
        lexer.getNextToken();
        exp = ParseExpression();
        if(!exp) {
            LogError("Illegal Value"); 
            return nullptr;
        }
        if (!(ty->doesMatch(exp->getType())))
        {
           
            LogError("Type Mismatch in Variable Declaration");
            return nullptr;
        }
    }
    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected ';' or '='");
        return nullptr;
    }
    if(!parsingFuncDef)return make_unique<GlobalVariableDeclaration>(move(var), move(exp));
    else return make_unique<LocalVariableDeclaration>(move(var), move(exp));
}

unique_ptr<Statement> Parser::ParseReturnStatement()
{
    lexer.getNextToken();
    if (lexer.isTokenSemiColon()){
        if(currentFuncType->isVoid()) return make_unique<Return>(nullptr);
    }
    else{
        auto exp = ParseExpression();
        if (exp) 
            if(currentFuncType->doesMatch(exp->getType()))
                return make_unique<Return>(move(exp));
    }
    LogError("Type mismatch in 'return' statement and return 'type'");
    return nullptr;
}

unique_ptr<Statement> Parser::ParseIfElseStatement()
{
    lexer.getNextToken();
    if (!lexer.isTokenLeftParen())
    {
        LogError("Expected a '(' befor expression");
        return nullptr;
    }
    lexer.getNextToken();
    auto Exp = ParseExpression();
    if(!Exp->getType()->isBool()){
        LogError("Illegal condition in 'if'");
        return nullptr;
    }
    if (!lexer.isTokenRightParen())
    {
        LogError("Expected a ')' after expression");
        return nullptr;
    }
    lexer.getNextToken();
    auto CmpStat = ParseCompoundStatement();
    lexer.getNextToken();
    if (!lexer.isTokenElse())
    {
        LogError("Expected a 'else' after 'if'");
        return nullptr;
    }
    lexer.getNextToken();
    auto elseCmpStat = ParseCompoundStatement();
    return make_unique<IfElseStatement>(move(Exp), move(CmpStat), move(elseCmpStat));
}

unique_ptr<VariableAssignment> Parser::VariableAssignmentStatementHelper(const string &name)
{
    ::Type* t1;
    if (LocalVarTable[name])
        t1 = LocalVarTable[name].get();
    else if (GlobalVarTable[name])
        t1 = GlobalVarTable[name].get();
    else
    {
        LogError("Undefined Variable");
        return nullptr;
    }
    auto var = make_unique<Variable>(name,t1->getNew());
    if (!lexer.isTokenAssignmentOp())
    {
        LogError("Missing '=' operator");
        return nullptr;
    }
    lexer.getNextToken();
    auto exp = ParseExpression();
    if(!t1->doesMatch(exp->getType()))
    {
        LogError("Type Mismatch");
        return nullptr;
    }
    return make_unique<VariableAssignment>(move(var), move(exp));
}

unique_ptr<Statement> Parser::ParseForStatement()
{
    unique_ptr<LocalVariableDeclaration> lvd;
    unique_ptr<VariableAssignment> va;
    unique_ptr<VariableAssignment> vastep;
    lexer.getNextToken();
    if (!lexer.isTokenLeftParen())
    {
        LogError("Expected a '(' in 'for'");
        return nullptr;
    }
    lexer.getNextToken();
    if (lexer.isTokenInt() || lexer.isTokenDouble())
    {
        auto rtl = ParseVariableDeclarationStatement();
        LocalVariableDeclaration* ptrlvd = (LocalVariableDeclaration*)rtl.release();
        lvd.reset(ptrlvd);
    }
    else if (lexer.isTokenIdentifier())
    {
        string Name = lexer.getIdentifier();
        lexer.getNextToken();
        va.reset(((VariableAssignment*)ParseVariableAssignmentStatement(Name).release()));
        lvd = nullptr;
    }
    else if (lexer.isTokenSemiColon())
    {
        lvd = nullptr;
        va = nullptr;
    }
    else
    {
        LogError("Unknown Statement in 'for'");
        return nullptr;
    }

    lexer.getNextToken();
    auto cond = ParseExpression();
    if (!cond || !cond->getType()->isBool())
    {
        LogError("Illegal condition in 'for'");
        return nullptr;
    }

    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected a ';' in 'for'");
        return nullptr;
    }

    lexer.getNextToken();
    if (lexer.isTokenIdentifier())
    {
        string Name = lexer.getIdentifier();
        lexer.getNextToken();
        vastep = VariableAssignmentStatementHelper(Name);
        if (!lexer.isTokenRightParen())
        {
            LogError("Expected a ')' in 'for'");
            return nullptr;
        }
    }
    else if (!lexer.isTokenRightParen())
    {
        LogError("Expected a ')' in 'for'");
        return nullptr;
    }
    lexer.getNextToken();
    auto cmpStat = ParseCompoundStatement();
    return make_unique<ForStatement>(move(lvd), move(va), move(cond), move(vastep), move(cmpStat));
}

unique_ptr<FunctionSignature> Parser::ParseConsume()
{
    lexer.getNextToken();
    if (!lexer.isTokenFunctionKeyword())
    {
        LogError("Expected keyword 'fn'");
        return nullptr;
    }
    lexer.getNextToken();
    auto fs = ParseFunctionSignature();
    lexer.getNextToken();
    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected a ';' after 'consume'");
        return nullptr;
    }
    return fs;
}

unique_ptr<FunctionSignature> Parser::ParseFunctionSignature()
{
    if (!lexer.isTokenIdentifier())
    {
        LogError("Expected an Identifier");
        return nullptr;
    }
    string Name = lexer.getIdentifier();
    if (FunctionTable.find(Name) != FunctionTable.end())
    {
        LogError("Function Already Defined");
        return nullptr;
    }
    vector<unique_ptr<::Type>> argType;
    lexer.getNextToken();
    if (!lexer.isTokenLeftParen())
    {
        LogError("Missing '(' in declaration");
        return nullptr;
    }
    lexer.getNextToken();
    vector<unique_ptr<Variable>> args;
    while (lexer.isAnyType())
    {
        unique_ptr<::Type> type = getType();
        lexer.getNextToken();
        if (!lexer.isTokenIdentifier())
        {
            LogError("Identifier Expected");
            return nullptr;
        }
        string name = lexer.getIdentifier();
        if (LocalVarTable[name])
        {
            LogError("Illegal Re-declaration");
            return nullptr;
        }
        LocalVarTable[name] = type->getNew();
        auto var = make_unique<Variable>(name,type->getNew());
        argType.push_back(move(type));
        args.push_back(move(var));
        lexer.getNextToken();
        if (lexer.isTokenRightParen())
            break;
        if (!lexer.isTokenComma())
        {
            LogError("Expected a ',' between arguments");
            return nullptr;
        }
        lexer.getNextToken();
    }
    if (!lexer.isTokenRightParen())
    {
        LogError("Missing ')' in declaration");
        return nullptr;
    }
    lexer.getNextToken();
    if (!lexer.isAnyType())
    {
        LogError("Incomplete Type Specification in Function Declaration");
        return nullptr;
    }
    unique_ptr<::Type> type = getType();
    currentFuncType = type.get();
    FunctionTable[Name] = make_pair(move(argType), move(type->getNew()));
    return make_unique<FunctionSignature>(move(Name), move(type), move(args));
}

unique_ptr<CompoundStatement> Parser::ParseCompoundStatement()
{
    if (!lexer.isTokenLeftCurlyBrace())
    {
        LogError("Missing '{' in Statement");
        return nullptr;
    }
    lexer.getNextToken();
    vector<unique_ptr<Statement>> statements;
    while (!lexer.isTokenRightCurlyBrace())
    {
        auto stat = ParseStatement();
        if (!stat)
        {
            LogError("Statement Might be missing '}'");
            return nullptr;
        }
        statements.push_back(move(stat));
        lexer.getNextToken();
    }
    return make_unique<CompoundStatement>(move(statements));
}

unique_ptr<FunctionDefinition> Parser::ParseFunctionDefinition()
{
    lexer.getNextToken();
    auto FH = ParseFunctionSignature();
    if (!FH)
        return nullptr;
    lexer.getNextToken();
    parsingFuncDef = true;
    auto CS = ParseCompoundStatement();
    if (!CS)
        return nullptr;
    if (!CS->isLastElementReturnStatement())
    {
        LogError("Missing 'return' statement at the end of Function Definition");
        return nullptr;
    }
    parsingFuncDef = false;
    LocalVarTable.clear();
    return make_unique<FunctionDefinition>(move(FH), move(CS));
}