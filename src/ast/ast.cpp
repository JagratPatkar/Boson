#include "ast.h"
#include "../codegen/codegen.h"
#include <vector>
#include "llvm/IR/Verifier.h"
using namespace std;
using namespace llvm;
using namespace AST;


static CodeGen* cg = CodeGen::GetInstance();


// map<std::string, GlobalVariable *> SymTab;
// map<std::string, AllocaInst *> SymTabLoc;

static bool generatingFunction = false;

Types AST::TypesOnToken(int type)
{
    if (type == -4)
        return type_int;
    if (type == -5)
        return type_double;
    if (type == -15)
        return type_void;
    return type_err;
}

const char *AST::TypesName(int t)
{
    if (t == -1)
        return "int";
    if (t == -2)
        return "double";
    if (t == -3)
        return "void";
    return "unknown type";
}

llvm::Value *IntNum::codeGen()
{
    Type *type = IntegerType::getInt32Ty(*(cg->context));
    return ConstantInt::get(type, Number, true);
}
llvm::Value *DoubleNum::codeGen()
{
    return ConstantFP::get(*(cg->context), APFloat(Number));
}

llvm::Value *FunctionCall::codeGen()
{
    Function *func = cg->module->getFunction(Name);
    vector<llvm::Value *> ArgsValue;
    for (auto i = args.begin(); i != args.end(); i++)
    {
        ArgsValue.push_back(i->get()->codeGen());
        if (!ArgsValue.back())
            return nullptr;
    }
    return cg->builder->CreateCall(func, ArgsValue, "callres");
}

void FunctionCall::VarDecCodeGen(GlobalVariable *gVar, Types)
{
    cg->builder->SetInsertPoint(cg->getCOPBB());
    GlobalVariable *gvar = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = codeGen();
    cg->builder->CreateAlignedStore(v, gVar, MaybeAlign(4));
}

llvm::Value *Variable::codeGen()
{
    if (generatingFunction)
    {
        llvm::Value *v = cg->LocalVarTable.getElement(Name);
        if (v)
            return cg->builder->CreateAlignedLoad(v, MaybeAlign(4), Name.c_str());
    }
    GlobalVariable *gVar = cg->GlobalVarTable.getElement(Name);
    if (!gVar)
        return nullptr;
    return cg->builder->CreateAlignedLoad(gVar, MaybeAlign(4), Name.c_str());
}

void Variable::VarDecCodeGen(GlobalVariable *gVar, Types)
{
    cg->builder->SetInsertPoint(cg->getCOPBB());
    GlobalVariable *gvar = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = cg->builder->CreateAlignedLoad(gvar, MaybeAlign(4), Name.c_str());
    cg->builder->CreateAlignedStore(v, gVar, MaybeAlign(4));
}

void AST::Value::VarDecCodeGen(GlobalVariable *gVar, Types)
{
    Constant *constant = dyn_cast<Constant>(codeGen());
    gVar->setInitializer(constant);
}

void GlobalVariableDeclaration::codegen()
{
    string Name = var->getName();
    Types vt = var->getType();
    if (vt == type_int)
    {
        cg->module->getOrInsertGlobal(Name, Type::getInt32Ty(*(cg->context)));
    }
    else
        cg->module->getOrInsertGlobal(Name, Type::getDoubleTy(*(cg->context)));
    GlobalVariable *gVar = cg->module->getNamedGlobal(Name);
    gVar->setAlignment(MaybeAlign(4));
    if (exp)
        exp->VarDecCodeGen(gVar, vt);
    else
    {
        if (vt == type_int)
            gVar->setInitializer(ConstantInt::get(Type::getInt32Ty(*(cg->context)), 0, true));
        else
            gVar->setInitializer(ConstantFP::get(*(cg->context), APFloat(0.0)));
    }
    cg->GlobalVarTable.addElement(Name,gVar);
}

void LocalVariableDeclaration::codegen()
{
    string Name = var->getName();
    Types vt = var->getType();

    llvm::Value *val;
    if (exp)
    {
        val = exp->codeGen();
    }
    else
    {
        if (vt == type_int)
            val = ConstantInt::get(IntegerType::getInt32Ty(*(cg->context)), 0, true);
        else
            val = ConstantFP::get(*(cg->context), APFloat(0.0));
    }
    AllocaInst *alloca;
    if (vt == type_int)
        alloca = new AllocaInst(IntegerType::getInt32Ty(*(cg->context)), 0, 0, Align(4), Name.c_str(), cg->builder->GetInsertBlock());
    else if (vt == type_double)
        alloca = cg->builder->CreateAlloca(cg->builder->getDoubleTy(), 0, Name.c_str());
    cg->LocalVarTable.addElement(Name,alloca);
    cg->builder->CreateAlignedStore(val, alloca, MaybeAlign(4));
}

void CompoundStatement::codegen()
{
    for (auto stat = Statements.begin(); stat != Statements.end(); stat++)
    {
        stat->get()->codegen();
    }
}

void VariableAssignment::codegen()
{
    string Name = var->getName();
    llvm::Value *val = exp->codeGen();
    if (!val)
        return;
    llvm::Value *dest = cg->LocalVarTable.getElement(Name);
    if (!dest)
    {
        GlobalVariable *globalDest = cg->GlobalVarTable.getElement(Name);
        if (!globalDest)
            return;
        cg->builder->CreateAlignedStore(val, globalDest, MaybeAlign(4));
    }
    else
    {
        cg->builder->CreateAlignedStore(val, dest, MaybeAlign(4));
    }
}

void Return::codegen()
{
    if (exp)
    {
        llvm::Value *v = exp->codeGen();
        if (!v)
            return;
        Types t = exp->getType();
        cg->builder->CreateRet(v);
    }
    else
    {
        cg->builder->CreateRetVoid();
    }
}

void FunctionCall::codegen()
{
    Function *func = cg->module->getFunction(Name);
    vector<llvm::Value *> ArgsValue;
    for (auto i = args.begin(); i != args.end(); i++)
    {
        ArgsValue.push_back(i->get()->codeGen());
        if (!ArgsValue.back())
            return;
    }
    cg->builder->CreateCall(func, ArgsValue);
}

void IfElseStatement::codegen()
{
    BasicBlock *bb = cg->builder->GetInsertBlock();
    Function *func = bb->getParent();
    llvm::Value *cmp = Condition->codeGen();
    cmp = cg->builder->CreateUIToFP(cmp, Type::getDoubleTy(*(cg->context)), "convtoFP");
    llvm::Value *cond = cg->builder->CreateFCmpUNE(cmp, ConstantFP::get(*(cg->context), APFloat(0.0)), "ifcond");
    BasicBlock *ThenBB = BasicBlock::Create(*(cg->context), "then", func);
    BasicBlock *ElseBB = BasicBlock::Create(*(cg->context), "else", func);
    BasicBlock *AfterIfElse = BasicBlock::Create(*(cg->context), "afterif", func);
    cg->builder->CreateCondBr(cond, ThenBB, ElseBB);
    cg->builder->SetInsertPoint(ThenBB);
    compoundStatements->codegen();
    cg->builder->CreateBr(AfterIfElse);
    cg->builder->SetInsertPoint(ElseBB);
    elseCompoundStatements->codegen();
    cg->builder->CreateBr(AfterIfElse);
    cg->builder->SetInsertPoint(AfterIfElse);
}

void ForStatement::codegen()
{
    BasicBlock *bb = cg->builder->GetInsertBlock();
    Function *func = bb->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(*(cg->context), "loop", func);
    BasicBlock *AfterLoopBB = BasicBlock::Create(*(cg->context), "afterloop", func);
    if (lvd)
        lvd->codegen();
    if (va)
        va->codegen();
    llvm::Value *condition = cond->codeGen();
    cg->builder->CreateCondBr(condition, LoopBB, AfterLoopBB);
    cg->builder->SetInsertPoint(LoopBB);
    compoundStatement->codegen();
    if (vastep)
        vastep->codegen();
    condition = cond->codeGen();
    cg->builder->CreateCondBr(condition, LoopBB, AfterLoopBB);
    cg->builder->SetInsertPoint(AfterLoopBB);
}

void BinaryExpression::VarDecCodeGen(GlobalVariable *gVar, Types vt)
{
    if (vt == type_int)
        gVar->setInitializer(ConstantInt::get(Type::getInt32Ty(*(cg->context)), 0, true));
    else
        gVar->setInitializer(ConstantFP::get(*(cg->context), APFloat(0.0)));
    cg->builder->SetInsertPoint(cg->getCOPBB());
    llvm::Value *v = codeGen();
    cg->builder->CreateAlignedStore(v, gVar, MaybeAlign(4));
}

llvm::Value *BinaryExpression::codeGen()
{
    llvm::Value *lhs = LVAL->codeGen();
    llvm::Value *rhs = RVAL->codeGen();

    if (LVAL->getType() == type_int)
    {
        switch (op)
        {
        case op_add:
            return cg->builder->CreateAdd(lhs, rhs, "additmp");
        case op_sub:
            return cg->builder->CreateSub(lhs, rhs, "subitmp");
        case op_mul:
            return cg->builder->CreateMul(lhs, rhs, "mulitmp");
        case op_div:
            return cg->builder->CreateSDiv(lhs, rhs, "divitmp");
        case op_less_than:
            return cg->builder->CreateICmpULT(lhs, rhs, "ltcmpi");
        case op_greater_than:
            return cg->builder->CreateICmpUGT(lhs, rhs, "gtcmpi");
        case op_less_than_eq:
            return cg->builder->CreateICmpULE(lhs, rhs, "lecmpi");
        case op_greater_than_eq:
            return cg->builder->CreateICmpUGE(lhs, rhs, "gecmpi");
        case op_equal_to:
            return cg->builder->CreateICmpEQ(lhs, rhs, "eqi");
        case op_not_equal_to:
            return cg->builder->CreateICmpNE(lhs, rhs, "neqcmpi");
        case non_op:
            return nullptr;
        }
    }

    switch (op)
    {
    case op_add:
        return cg->builder->CreateFAdd(lhs, rhs, "addftmp");
    case op_sub:
        return cg->builder->CreateFSub(lhs, rhs, "subftmp");
    case op_mul:
        return cg->builder->CreateFMul(lhs, rhs, "mulftmp");
    case op_div:
        return cg->builder->CreateFDiv(lhs, rhs, "divftmp");
    case op_less_than:
        return cg->builder->CreateFCmpULT(lhs, rhs, "ltcmpd");
    case op_greater_than:
        return cg->builder->CreateFCmpUGT(lhs, rhs, "gtcmpd");
    case op_less_than_eq:
        return cg->builder->CreateFCmpULE(lhs, rhs, "lecmpd");
    case op_greater_than_eq:
        return cg->builder->CreateFCmpUGE(lhs, rhs, "gecmpd");
    case op_equal_to:
        return cg->builder->CreateFCmpUEQ(lhs, rhs, "eqcmpd");
    case op_not_equal_to:
        return cg->builder->CreateFCmpUNE(lhs, rhs, "neqcmpd");
    case non_op:
        return nullptr;
    }
    return nullptr;
}

void FunctionSignature::codegen()
{
    FunctionType *funcType;
    vector<Type *> Args;
    for (auto i = args.begin(); i != args.end(); i++)
    {
        Types t = i->get()->getType();
        if (t == type_int)
            Args.push_back(cg->builder->getInt32Ty());
        else
            Args.push_back(cg->builder->getDoubleTy());
    }
    if (getRetType() == type_int)
        funcType = FunctionType::get(cg->builder->getInt32Ty(), Args, false);
    else if (getRetType() == type_double)
        funcType = FunctionType::get(cg->builder->getDoubleTy(), Args, false);
    else if (getRetType() == type_void)
        funcType = FunctionType::get(cg->builder->getVoidTy(), Args, false);
    Function *function = Function::Create(funcType, Function::ExternalLinkage, getName(), *(cg->module));
}

void FunctionDefinition::codeGen()
{
    generatingFunction = true;
    functionSignature->codegen();
    Function *function = cg->module->getFunction(functionSignature->getName());
    BasicBlock *bb = BasicBlock::Create(*(cg->context), "entry", function);
    cg->builder->SetInsertPoint(bb);
    if (functionSignature->getName() == "start")
    {
        vector<llvm::Value *> args;
        cg->builder->CreateCall(cg->getCOPFP(), args);
    }
    Function::arg_iterator ai, ae;
    auto n = functionSignature->args.begin();
    for (ai = function->arg_begin(), ae = function->arg_end(); ai != ae; ++ai, ++n)
    {
        string name = n->get()->getName();
        Types t = n->get()->getType();
        AllocaInst *alloca;
        if (t == type_int)
            alloca = new AllocaInst(IntegerType::getInt32Ty(*(cg->context)), 0, 0, Align(4), name, cg->builder->GetInsertBlock());
        else
            alloca = cg->builder->CreateAlloca(cg->builder->getDoubleTy(), 0, name);
        cg->builder->CreateAlignedStore(ai, alloca, MaybeAlign(4));
        cg->LocalVarTable.addElement(name, alloca);
        ai->setName(name);
    }
    compoundStatements->codegen();
    generatingFunction = false;
    cg->LocalVarTable.clearTable();
    if (verifyFunction(*function))
    {
        string str = functionSignature->getName();
        printf("Error in %s \n", str.c_str());
    }
}