#include "parser.h"

#include "llvm/IR/Verifier.h"

static CodeGen *cg = CodeGen::GetInstance();

void Parser::parse()
{
    lexer.getNextToken();
    while (true)
    {
        if (lexer.isTokenIdentifier())
            ParseStatementIdentifier()->codegen();
        else if (lexer.isTokenInt() || lexer.isTokenDouble() || lexer.isTokenBoolean() || lexer.isTokenObj())
            ParseVariableDeclarationStatement()->codegen();
        else if (lexer.isTokenFunctionKeyword())
            ParseFunctionDefinition()->codeGen();
        else if (lexer.isTokenConsume())
            ParseConsume()->codegen();
        else
        {
            // Log an error for unexpected tokens
            if (!lexer.isTokenEOF())
            {
                LogError("Unexpected token encountered");
            }
            else
            {
                if (FunctionTable.find("start") == FunctionTable.end())
                {
                    LogError("Start Function Not Found");
                }

                cg->terminateCOPBB();
                cg->terminateCFBB();
                lexer.closeFile();
            }

            return;
        }
        lexer.getNextToken();
    }
}

unique_ptr<Expression> Parser::ParsePreUnary()
{
    bool flag = false;
    unique_ptr<UnOps> unop;
    if (lexer.isTokenIncrement())
    {
        unop = make_unique<AddPreIncrement>();
        flag = true;
    }
    else if (lexer.isTokenDecrement())
    {
        unop = make_unique<SubPreIncrement>();
        flag = true;
    }
    else if (lexer.isTokenNot())
    {
        unop = make_unique<PreNot>();
        flag = true;
    }
    else if (lexer.isTokenSubSym())
    {
        unop = make_unique<Neg>();
        flag = true;
    }

    if (flag)
    {
        lexer.getNextToken();
        auto e = ParsePrimary();
        if (!e)
        {
            LogError("Invalid use of operator");
        }
        if (!unop->validOperandSet(e.get()))
        {
            LogError("Cannot Apply operator to this type");
            return nullptr;
        }

        auto ty = unop->getOperatorEvalTy();

        return make_unique<UnaryExpression>(move(unop), move(e), move(ty));
    }

    return ParsePrimary();
}

unique_ptr<Expression> Parser::ParsePostUnary(unique_ptr<Expression> e)
{
    bool flag = false;
    unique_ptr<UnOps> unop;
    if (lexer.isTokenIncrement())
    {
        unop = make_unique<AddPostIncrement>();
        flag = true;
    }
    else if (lexer.isTokenDecrement())
    {
        unop = make_unique<SubPostIncrement>();
        flag = true;
    }

    if (flag)
    {
        if (!unop->validOperandSet(e.get()))
        {
            LogError("Cannot Apply operator to non-variable element");
            return nullptr;
        }
        lexer.getNextToken();
        auto ty = unop->getOperatorEvalTy();
        return make_unique<UnaryExpression>(move(unop), move(e), move(ty));
    }

    return move(e);
}

unique_ptr<Expression> Parser::ParseExpression()
{
    auto lvalue = ParsePostUnary(ParsePreUnary());
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
    if (lexer.isTokenTrueValue() || lexer.isTokenFalseValue())
        return ParseBooleanValue();
    if (lexer.isTokenIdentifier())
    {
        return ParseIdentifier();
    }

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
    ::Type *t;
    if (parsingFuncDef && LocalVarTable[Name])
    {
        t = LocalVarTable[Name].get();
    }
    else if (GlobalVarTable[Name])
    {
        t = GlobalVarTable[Name].get();
    }
    else
    {
        LogError("Undefined Variable");
        return nullptr;
    }
    return make_unique<Variable>(Name, t->getNew());
}

unique_ptr<Expression> Parser::ParseIdentifier()
{
    string Name = lexer.getIdentifier();
    lexer.getNextToken();
    if (lexer.isTokenLeftParen())
        return ParseCallExpression(Name);
    else if (lexer.isTokenLeftSquareBrack())
        return ParseArrayElemExpression(Name);
    else if (lexer.isTokenDot())
        return ParseObjectMemberExpression(Name);
    else
        return ParseVariable(Name);
}

unique_ptr<Expression> Parser::ParseObjectMemberExpression(const string &Name)
{
    ::Type *t;
    if (parsingFuncDef && LocalVarTable[Name])
    {
        t = LocalVarTable[Name].get();
    }
    else if (GlobalVarTable[Name])
    {
        t = GlobalVarTable[Name].get();
    }
    else
    {
        LogError("Undefined Object!");
        return nullptr;
    }

    lexer.getNextToken();

    if (!lexer.isTokenIdentifier())
    {
        LogError("Expected a member name after '.'");
        return nullptr;
    }

    lexer.getNextToken();

    unique_ptr<Variable> v;
    int elemNum;
    if (t->isObject())
    {
        auto op = static_cast<ObjectTy *>(t);
        string memberName = lexer.getIdentifier();
        auto mt = op->getProperty(memberName);
        elemNum = std::get<0>(mt);
        if (elemNum == -1)
        {
            LogError("No member by the name in Object");
            return nullptr;
        }
        v = make_unique<Variable>(Name, move(std::get<1>(mt)));
        v->setObjectType(t->getNew());
    }
    else
    {
        LogError("Primitive Variables don't have members!");
        return nullptr;
    }

    v->setElementNumber(elemNum);
    v->setObjElementFlag();
    return move(v);
}

unique_ptr<Expression> Parser::ParseArrayElemExpression(const string &Name)
{
    lexer.getNextToken();
    auto E = ParseExpression();

    if (!lexer.isTokenRightSquareBrack())
    {
        LogError("Expected a ']' after index");
        return nullptr;
    }

    lexer.getNextToken();

    unique_ptr<Variable> v;
    if (!((parsingFuncDef && LocalVarTable[Name]) || GlobalVarTable[Name]))
    {
        LogError("Undefined Array Variable");
        return nullptr;
    }
     ::Type *t = nullptr;
    if(LocalVarTable[Name].get()){
        t = LocalVarTable[Name].get();
    }
    else{
        t = GlobalVarTable[Name].get();
    }
    if (t->isArray())
    {
        v = make_unique<Variable>(Name, static_cast<Array *>(t)->getOfType()->getNew());
        v->setArrayType(t->getNew());
    }
    else{
        LogError("Cannot index a Primitive Variable");
        return nullptr;
    }

    v->setElement(move(E));
    v->setArrayFlag();
    return move(v);
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

unique_ptr<Expression> Parser::ParseBooleanValue()
{
    if (lexer.isTokenTrueValue())
    {
        lexer.getNextToken();
        return make_unique<Boolean>(true);
    }
    else if (lexer.isTokenFalseValue())
    {
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

unique_ptr<::Type> Parser::getType()
{
    unique_ptr<::Type> a;
    if (lexer.isTokenInt())
    {
        a = make_unique<Int>();
        lexer.getNextToken();
        if (lexer.isTokenLeftSquareBrack())
        {
            return ParseArrayType(move(a));
        }
    }
    else if (lexer.isTokenDouble())
    {
        a = make_unique<Double>();
        lexer.getNextToken();
        if (lexer.isTokenLeftSquareBrack())
        {
            return ParseArrayType(move(a));
        }
    }
    else if (lexer.isTokenVoid())
    {
        a = make_unique<Void>();
        lexer.getNextToken();
    }
    else if (lexer.isTokenBoolean())
    {
        a = make_unique<Bool>();
        lexer.getNextToken();
        if (lexer.isTokenLeftSquareBrack())
        {
            return ParseArrayType(move(a));
        }
    }
    else if (lexer.isTokenObj())
    {
        return ParseObjectType();
    }
    return a;
}

unique_ptr<::Type> Parser::ParseArrayType(unique_ptr<::Type> et)
{
    lexer.getNextToken();

    if (!lexer.isTokenIntNum())
    {
        LogError("Expected Length Specification of Array");
        return nullptr;
    }

    int num = lexer.getIntNum();

    lexer.getNextToken();
    if (!lexer.isTokenRightSquareBrack())
    {
        LogError("Expected a ']' in array declaration");
        return nullptr;
    }

    lexer.getNextToken();

    return make_unique<Array>(num, move(et));
}

unique_ptr<::Type> Parser::ParseObjectType()
{
    lexer.getNextToken();
    unique_ptr<::Type> t = make_unique<ObjectTy>();
    if (lexer.isTokenColon())
    {
        lexer.getNextToken();
        if (!lexer.isTokenLeftCurlyBrace())
        {
            LogError("Error in Object declaration missing '{'");
            return nullptr;
        }

        lexer.getNextToken();
        bool hasTypeMembers = false;
        vector<std::string> nameList;
        int counter = 0;
        while (!lexer.isTokenRightCurlyBrace())
        {
            hasTypeMembers = true;
            counter++;
            unique_ptr<::Type> ty = getType();
            if (!ty)
            {
                LogError("Object member needs a type");
                return nullptr;
            }
            if (ty->isObject() || ty->isArray())
            {
                LogError("Object conatining another Object or Array not yet supported!");
                return nullptr;
            }
            if (!lexer.isTokenIdentifier() && !parsingRetTy)
            {
                LogError("Identifier Expected in Object member declaration");
                return nullptr;
            }
            string memberName;
            if (parsingRetTy)
            {
                memberName = std::to_string(counter);
            }
            else
            {
                memberName = lexer.getIdentifier();
                if (std::find(nameList.begin(), nameList.end(), memberName) != nameList.end())
                {
                    LogError("Cannot reinitialize object members in object declaration");
                    return nullptr;
                }
                lexer.getNextToken();
            }
            nameList.push_back(memberName);

            dynamic_cast<ObjectTy *>(t.get())->setProperty(memberName, move(ty));

            if (!lexer.isTokenComma() && !lexer.isTokenRightCurlyBrace())
            {
                LogError("Expected a ',' between object member types or '}' and at the end of Object Type Declaration");
                return nullptr;
            }
            if (lexer.isTokenComma())
                lexer.getNextToken();
        }
        if (!hasTypeMembers)
        {
            LogError("Expected members in Object Declaration");
            return nullptr;
        }
        if (!lexer.isTokenRightCurlyBrace())
        {
            LogError("Expected a '}' at the end of Object Declaration");
            return nullptr;
        }
        dynamic_cast<ObjectTy *>(t.get())->createType();
        lexer.getNextToken();
    }

    return t;
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
        auto BinOp = returnBinOpsType();
        lexer.getNextToken();
        auto rvalue = ParsePostUnary(ParsePreUnary());
        if (!rvalue)
            return nullptr;
        auto ltype = lvalue->getType();
        auto rtype = rvalue->getType();
        if (!ltype->doesMatch(rtype))
        {
            LogError("Type Error");
            return nullptr;
        }
        if (!BinOp->validOperandSet(ltype))
        {
            LogError("Type of Operands given to Operator is Illeagal");
            return nullptr;
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
    while (true && !lexer.isTokenRightParen())
    {
        auto Exp = ParseExpression();
        if (!Exp)
        {
            LogError("Expected an Expression in Function Call!");
            return nullptr;
        }
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
        if (!a->get()->doesMatch(b->get()))
        {
            LogError("Type Mismatch in function call");
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
    if (lexer.isTokenInt() || lexer.isTokenDouble() || lexer.isTokenBoolean() || lexer.isTokenObj())
        return ParseVariableDeclarationStatement();
    if (lexer.isTokenReturnKeyword())
        return ParseReturnStatement();
    if (lexer.isTokenIf())
        return ParseIfElseStatement();
    if (lexer.isTokenFor())
        return ParseForStatement();
    if (lexer.isTokenForeach())
        return ParseForEach();
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
    {
        if (!lexer.isTokenAssignmentOp() && !lexer.isTokenDot() && !lexer.isTokenLeftSquareBrack())
        {
            LogError("Unknown statement!");
            return nullptr;
        }
        return ParseVariableAssignmentStatement(Name);
    }
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
    while (true && !lexer.isTokenRightParen())
    {
        auto Exp = ParseExpression();
        if (!Exp)
        {
            LogError("Expected an Expression in Function Call!");
            return nullptr;
        }
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
        if (!a->get()->doesMatch(b->get()))
        {
            LogError("Type Mismatch in function call");
        }
    }
    bool dalloc = false;
    auto sec = p.second.get()->getNew();
    if (sec->isObject() || sec->isArray())
    {
        dalloc = true;
    }
    FunctionTable[name] = move(p);
    auto fc = make_unique<FunctionCall>(name, move(Args), move(sec));
    fc->setDalloc(dalloc);
    if (!parsingFuncDef)
        fc->setGlobally();
    return fc;
}

unique_ptr<Statement> Parser::ParseArrayVariableDeclarationStatement(const string &name, unique_ptr<::Type> t)
{
    Array *ty = dynamic_cast<Array *>(t.get());
    vector<unique_ptr<Expression>> Args;
    vector<unique_ptr<::Type>> ArgType;
    unique_ptr<Expression> e;
    bool expFlag = false;
    if (lexer.isTokenAssignmentOp())
    {
        lexer.getNextToken();
        if (lexer.isTokenLeftSquareBrack())
        {
            lexer.getNextToken();
            while (true)
            {
                auto Exp = ParseExpression();
                if (!Exp)
                {
                    LogError("Expected an expression in Array Declaration");
                    return nullptr;
                }
                if (!lexer.isTokenComma() && !lexer.isTokenRightSquareBrack())
                {
                    LogError("Statement might be missing a ','  or a ']'");
                    return nullptr;
                }
                ArgType.push_back(Exp->getType()->getNew());
                Args.push_back(move(Exp));
                if (!lexer.isTokenRightSquareBrack())
                {
                    lexer.getNextToken();
                }
                else
                    break;
            }
            lexer.getNextToken();
            if (Args.size() != ty->getSize())
            {
                LogError("Array is initiaized with illegal number of elements");
                return nullptr;
            }
        }
        else
        {
            expFlag = true;
            e = ParseExpression();
            if (!e)
            {
                LogError("Expected an array literal or expression in array declaration");
                return nullptr;
            }
            if (!e->getType()->isArray())
            {
                LogError("Expected an expression of array type in array declaration");
                return nullptr;
            }
            if (!e->getType()->doesMatch(ty))
            {
                LogError("Type mismatch in array declaration");
                return nullptr;
            }
        }
    }
    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected a ';' at the end of statement");
        return nullptr;
    }
    for (auto i = ArgType.begin(); i != ArgType.end(); i++)
    {
        if (!i->get()->doesMatch(ty->getOfType()))
        {
            LogError("Type Mismatch connot assign this value to array");
            return nullptr;
        }
    }
    unique_ptr<ArrayVal> av;
    ty->setName(name);
    if (expFlag)
    {
        av = make_unique<ArrayVal>(move(e), move(t), name);
    }
    else
    {
        av = make_unique<ArrayVal>(move(Args), move(t), name);
    }

    auto var = make_unique<Variable>(name, move(av->getType()->getNew()));
    if (!parsingFuncDef)
    {
        GlobalVarTable[name] = av->getType()->getNew();
        return make_unique<GlobalVariableDeclaration>(move(var), move(av));
    }

    LocalVarTable[name] = av->getType()->getNew();
    return make_unique<LocalVariableDeclaration>(move(var), move(av));
}

unique_ptr<Statement> Parser::ParseObjectVariableDeclarationStatement(const string &name, unique_ptr<::Type> t)
{

    if (!lexer.isTokenAssignmentOp())
    {
        LogError("Expected a '=' in object declaration");
        return nullptr;
    }
    unique_ptr<Expression> e;
    lexer.getNextToken();
    bool expFlag = false;
    vector<std::string> nameList;
    vector<unique_ptr<ObjMember>> memberList;
    if (lexer.isTokenLeftCurlyBrace())
    {
        lexer.getNextToken();

        while (!lexer.isTokenRightCurlyBrace())
        {
            unique_ptr<::Type> ty = getType();
            if (!ty)
            {
                LogError("Object member needs a type");
                return nullptr;
            }
            if (ty->isObject() || ty->isArray())
            {
                LogError("Object conatining another Object or Array not yet supported!");
                return nullptr;
            }

            if (!lexer.isTokenIdentifier())
            {
                LogError("Identifier Expected in Object member declaration");
                return nullptr;
            }

            string memberName = lexer.getIdentifier();
            if (std::find(nameList.begin(), nameList.end(), memberName) != nameList.end())
            {
                LogError("Cannot reinitialize object members in object declaration");
                return nullptr;
            }
            nameList.push_back(memberName);
            lexer.getNextToken();
            unique_ptr<Expression> Exp;
            if (lexer.isTokenAssignmentOp())
            {
                lexer.getNextToken();
                Exp = ParseExpression();
                if (!Exp)
                {
                    LogError("Expected an expression for object member initilization");
                    return nullptr;
                }
                if (!Exp->getType()->doesMatch(ty.get()))
                {
                    LogError("Typemismatch in object member");
                    return nullptr;
                }
            }

            dynamic_cast<ObjectTy *>(t.get())->setProperty(memberName, move(ty));
            memberList.push_back(make_unique<ObjMember>(memberName, move(Exp)));

            if (!lexer.isTokenComma() && !lexer.isTokenRightCurlyBrace())
            {
                LogError("Expected a ',' between object members or '}' and at the end of Object Declaration");
                return nullptr;
            }
            if (lexer.isTokenComma())
                lexer.getNextToken();
        }

        if (!lexer.isTokenRightCurlyBrace())
        {
            LogError("Expected a '}' at the end of Object Declaration");
            return nullptr;
        }
        lexer.getNextToken();
    }
    else
    {
        expFlag = true;
        e = ParseExpression();
        if (!e)
        {
            LogError("Expected an object literal or expression in object declaration");
            return nullptr;
        }
        if (!e->getType()->isObject())
        {
            LogError("Expected an expression of object type in object declaration");
            return nullptr;
        }
        ObjectTy *pt = dynamic_cast<ObjectTy *>(e->getType());
        t = e->getType()->getNew();
        dynamic_cast<ObjectTy *>(t.get())->setName(name);
    }
    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected a ';' at the end of statement");
        return nullptr;
    }
    unique_ptr<ObjLiteral> av;
    if (!expFlag)
    {
        ObjectTy *pt = dynamic_cast<ObjectTy *>(t.get());
        pt->setName(name);
        pt->createType();
        av = make_unique<ObjLiteral>(move(memberList), move(t));
    }
    else
    {
        av = make_unique<ObjLiteral>(move(e), move(t));
    }

    auto var = make_unique<Variable>(name, move(av->getType()->getNew()));
    if (!parsingFuncDef)
    {
        GlobalVarTable[name] = av->getType()->getNew();
        return make_unique<GlobalVariableDeclaration>(move(var), move(av));
    }

    LocalVarTable[name] = av->getType()->getNew();
    return make_unique<LocalVariableDeclaration>(move(var), move(av));
}

unique_ptr<Statement> Parser::ParseVariableDeclarationStatement()
{
    unique_ptr<::Type> ty = getType();
    string name;
    if (!lexer.isTokenIdentifier())
    {
        LogError("Identifier Expected");
        return nullptr;
    }
    name = lexer.getIdentifier();
    lexer.getNextToken();

    if (parsingFuncDef)
    {
        if (LocalVarTable[name])
        {
            LogError("Illegal Re-declaration");
            return nullptr;
        }
    }
    else if (GlobalVarTable[name])
    {
        LogError("Illegal Re-declaration");
        return nullptr;
    }
    else if (FunctionTable.find(name) != FunctionTable.end())
    {
        LogError("Illegal Re-declaration");
        return nullptr;
    }

    if (ty->isObject())
    {
        return ParseObjectVariableDeclarationStatement(name, move(ty));
    }

    if (ty->isArray())
    {
        return ParseArrayVariableDeclarationStatement(name, move(ty));
    }

    if (!parsingFuncDef)
        GlobalVarTable[name] = ty->getNew();
    else
        LocalVarTable[name] = ty->getNew();
    auto var = make_unique<Variable>(name, move(ty->getNew()));

    unique_ptr<Expression> exp = nullptr;
    if (lexer.isTokenAssignmentOp())
    {
        lexer.getNextToken();
        exp = ParseExpression();
        if (!exp)
        {
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
    if (!parsingFuncDef)
        return make_unique<GlobalVariableDeclaration>(move(var), move(exp));
    else
        return make_unique<LocalVariableDeclaration>(move(var), move(exp));
}

unique_ptr<Statement> Parser::ParseReturnStatement()
{
    lexer.getNextToken();
    if (lexer.isTokenSemiColon())
    {
        if (currentFuncType->isVoid())
            return make_unique<Return>(nullptr);
    }
    else
    {
        auto exp = ParseExpression();
        if (exp)
        {
            if (currentFuncType->doesMatch(exp->getType()))
            {
                return make_unique<Return>(move(exp));
            }
        }
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
    if (!Exp->getType()->isBool())
    {
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
    if (!CmpStat)
        return nullptr;
    lexer.getNextToken();
    if (lexer.isTokenElse())
    {
        lexer.getNextToken();
        auto elseCmpStat = ParseCompoundStatement();
        if (!elseCmpStat)
            return nullptr;
        auto rt = make_unique<IfElseStatement>(move(Exp), move(CmpStat), move(elseCmpStat));
        rt->peekedOneAhead(false);
        return move(rt);
    }
    auto rt = make_unique<IfElseStatement>(move(Exp), move(CmpStat), nullptr);
    rt->peekedOneAhead(true);
    return move(rt);
}

unique_ptr<VariableAssignment> Parser::VariableAssignmentStatementHelper(const string &name)
{
    ::Type *t1;
    if (LocalVarTable[name])
        t1 = LocalVarTable[name].get();
    else if (GlobalVarTable[name])
        t1 = GlobalVarTable[name].get();
    else
    {
        LogError("Undefined Variable");
        return nullptr;
    }

    unique_ptr<Variable> var;
    if (lexer.isTokenLeftSquareBrack())
    {
        auto et = dynamic_cast<Array *>(t1);
        var = make_unique<Variable>(name, et->getOfType()->getNew());
        lexer.getNextToken();
        auto E = ParseExpression();
        var->setElement(move(E));
        var->setArrayFlag();
        var->setArrayType(t1->getNew());
        if (!lexer.isTokenRightSquareBrack())
        {
            LogError("Expexted a ']' after index");
        }
        lexer.getNextToken();
    }
    else if (lexer.isTokenDot() && t1->isObject())
    {
        lexer.getNextToken();
        if (!lexer.isTokenIdentifier())
        {
            LogError("Expected a member name after '.'");
            return nullptr;
        }

        int elemNum;
        auto op = static_cast<ObjectTy *>(t1);
        string memberName = lexer.getIdentifier();
        auto mt = op->getProperty(memberName);
        elemNum = std::get<0>(mt);
        if (elemNum == -1)
        {
            LogError("No member by the name in Object");
            return nullptr;
        }
        var = make_unique<Variable>(name, move(std::get<1>(mt)));
        var->setObjectType(t1->getNew());
        var->setObjElementFlag();
        var->setElementNumber(elemNum);
        lexer.getNextToken();
    }
    else if (!t1->isObject() && lexer.isTokenDot())
    {
        LogError("Primitive Variables don't have members!");
        return nullptr;
    }
    else
    {
        var = make_unique<Variable>(name, t1->getNew());
        var->setElement(0);
    }

    if (!lexer.isTokenAssignmentOp())
    {
        LogError("Missing '=' operator");
        return nullptr;
    }
    lexer.getNextToken();
    auto exp = ParseExpression();
    if (!var->getType()->doesMatch(exp->getType()))
    {
        LogError("Type Mismatch in variable assignment");
        return nullptr;
    }
    if (var->getType()->isObject())
    {
        if (LocalVarTable[var->getName()])
        {
            LocalVarTable[var->getName()] = exp->getType()->getNew();
        }
        else if (GlobalVarTable[var->getName()])
        {
            GlobalVarTable[var->getName()] = exp->getType()->getNew();
        }
    }
    auto va = make_unique<VariableAssignment>(move(var), move(exp));
    if (!parsingFuncDef)
        va->setGlobally();
    return va;
}

unique_ptr<Statement> Parser::ParseForEach(){
    lexer.getNextToken();

    if (!lexer.isTokenLeftParen())
    {
        LogError("Expected a '(' in 'foreach'");
        return nullptr;
    }

    lexer.getNextToken();

    if (!lexer.isTokenIdentifier()){
        LogError("Expected a 'variable name' in 'foreach'");
        return nullptr;
    }

    string varName = lexer.getIdentifier();

    if (LocalVarTable[varName] || GlobalVarTable[varName])
    {
        LogError("Variable declared in 'foreach' already exist, change the name.");
        return nullptr;
    }

    lexer.getNextToken();

    if(!lexer.isTokenAs()){
        LogError("Expected 'as' keyword in 'foreach'");
        return nullptr;
    }

    lexer.getNextToken();

    if (!lexer.isTokenIdentifier()){
        LogError("Expected a name for an array to loop on in 'foreach'");
        return nullptr;
    }

    string sourceName = lexer.getIdentifier();
     ::Type *t1;
    if (LocalVarTable[sourceName])
        t1 = LocalVarTable[sourceName].get();
    else if (GlobalVarTable[sourceName])
        t1 = GlobalVarTable[sourceName].get();
    else
    {
        LogError("Undefined Variable");
        return nullptr;
    }

    if(!t1->isArray()){
        LogError("The source variable in the 'foreach' should be an array");
        return nullptr;
    }

    Array* ty = static_cast<Array*>(t1);

    if (!parsingFuncDef)
        GlobalVarTable[varName] = ty->getOfType()->getNew();
    else
        LocalVarTable[varName] = ty->getOfType()->getNew();
    
    foreachNameList.push_back(varName);
    lexer.getNextToken();
    
    if (!lexer.isTokenRightParen())
    {
        LogError("Expected a ')' in 'for'");
        return nullptr;
    }
    lexer.getNextToken();
    auto cmpStat = ParseCompoundStatement();
    if (!cmpStat)
        return nullptr;
   
    
    return make_unique<ForEachStatement>(varName, sourceName, ty->getOfType()->getNew(), t1->getNew(),move(cmpStat));
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
        LocalVariableDeclaration *ptrlvd = (LocalVariableDeclaration *)rtl.release();
        lvd.reset(ptrlvd);
    }
    else if (lexer.isTokenIdentifier())
    {
        string Name = lexer.getIdentifier();
        lexer.getNextToken();
        va.reset(((VariableAssignment *)ParseVariableAssignmentStatement(Name).release()));
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
    if (!cmpStat)
        return nullptr;
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
    if (!lexer.isTokenSemiColon())
    {
        LogError("Expected a ';' after 'consume'");
        return nullptr;
    }
    LocalVarTable.clear();
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
    funcName = Name;
    if (FunctionTable.find(Name) != FunctionTable.end())
    {
        LogError("Function Already Defined");
        return nullptr;
    }
    else if (LocalVarTable[Name] || GlobalVarTable[Name])
    {
        LogError("Cannot have a 'function' and a 'variable' with the same name");
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
        string name;
        if (!lexer.isTokenIdentifier())
        {
            LogError("Identifier Expected");
            return nullptr;
        }
        name = lexer.getIdentifier();
        lexer.getNextToken();

        if (LocalVarTable[name])
        {
            LogError("Illegal Re-declaration");
            return nullptr;
        }
        LocalVarTable[name] = type->getNew();
        auto var = make_unique<Variable>(name, type->getNew());
        argType.push_back(move(type));
        args.push_back(move(var));
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
    parsingRetTy = true;
    unique_ptr<::Type> type = getType();
    parsingRetTy = false;
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
            LogError("Statement might be Missing '}'");
            return nullptr;
        }
        if (!stat->didPeekOneAhead())
            lexer.getNextToken();
        statements.push_back(move(stat));
    }
    return make_unique<CompoundStatement>(move(statements));
}

unique_ptr<FunctionDefinition> Parser::ParseFunctionDefinition()
{
    lexer.getNextToken();
    auto FH = ParseFunctionSignature();
    if (!FH)
        return nullptr;
    parsingFuncDef = true;
    auto CS = ParseCompoundStatement();
    if (!CS)
        return nullptr;
    if (!CS->isLastElementReturnStatement())
    {
        LogError("Missing 'return' statement at the end of Function Definition");
        return nullptr;
    }
    auto ty = CS->getReturnExpressionType()->getNew();
    auto fs = move(FunctionTable[funcName]);
    FunctionTable[funcName] = make_pair(move(fs.first), move(ty));
    parsingFuncDef = false;
    LocalVarTable.clear();
    return make_unique<FunctionDefinition>(move(FH), move(CS));
}