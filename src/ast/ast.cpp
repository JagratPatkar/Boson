#include "ast.h"
#include "../codegen/codegen.h"
#include <vector>
#include "llvm/IR/Verifier.h"
using namespace std;
using namespace llvm;
using namespace AST;

static CodeGen *cg = CodeGen::GetInstance();

llvm::Value *ArrayVal::codeGen()
{
    return nullptr;
}

void ArrayVal::gen(llvm::Value *vt)
{
    int counter = 0;
    for (auto j = ofVals.begin(); j != ofVals.end(); j++, counter++)
    {
        llvm::Value *v = j->get()->codeGen();
        type->createWrite(ConstantInt::get(llvm::Type::getInt32Ty(*(cg->context)), counter), v, vt);
    }
}

void ArrayVal::VarDecCodeGen(GlobalVariable *gVar, ::Type *t)
{
    bool flag = true;
    vector<Constant *> vals;
    for (auto i = ofVals.begin(); i != ofVals.end(); i++)
    {
        if (!i->get()->isValue())
        {
            flag = false;
            break;
        }
    }
    if (flag)
    {
        for (auto i = ofVals.begin(); i != ofVals.end(); i++)
        {
            vals.push_back(dyn_cast<Constant>(i->get()->codeGen()));
        }
        gVar->setInitializer(ConstantArray::get(dyn_cast<ArrayType>(type->getLLVMType()), vals));
    }
    else
    {
        cg->builder->SetInsertPoint(cg->getCOPBB());
        gen(gVar);
        gVar->setInitializer(type->getDefaultConstant());
    }
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

void FunctionCall::VarDecCodeGen(GlobalVariable *gVar, ::Type *t)
{
    cg->builder->SetInsertPoint(cg->getCOPBB());
    GlobalVariable *gvar = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = codeGen();
    cg->builder->CreateAlignedStore(v, gVar, t->getAllignment());
}

llvm::Value *Variable::codeGen()
{
    if (cg->getGeneratingFunction())
    {
        llvm::Value *v = cg->LocalVarTable.getElement(Name);
        if (isArrayElem && v)
        {
            return arrayType->createLoad(getElement(), v, Name);
        }
        else if (v)
        {
            return type->createLoad(0, v, Name);
        }
    }
    GlobalVariable *gVar = cg->GlobalVarTable.getElement(Name);
    if (!gVar)
        return nullptr;
    if (isArrayElem)
    {
        return arrayType->createLoad(getElement(), gVar, Name);
    }
    return type->createLoad(0, gVar, Name);
}

void Variable::VarDecCodeGen(GlobalVariable *gVar, ::Type *t)
{
    cg->builder->SetInsertPoint(cg->getCOPBB());
    GlobalVariable *gvar = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = cg->builder->CreateAlignedLoad(gvar, t->getAllignment(), Name.c_str());
    cg->builder->CreateAlignedStore(v, gVar, t->getAllignment());
}

void AST::Value::VarDecCodeGen(GlobalVariable *gVar, ::Type *)
{
    Constant *constant = dyn_cast<Constant>(codeGen());
    gVar->setInitializer(constant);
}

void GlobalVariableDeclaration::codegen()
{
    string Name = var->getName();
    ::Type *vt = var->getType();
    cg->module->getOrInsertGlobal(Name, vt->getLLVMType());
    GlobalVariable *gVar = cg->module->getNamedGlobal(Name);
    gVar->setAlignment(vt->getAllignment());
    if (exp)
        exp->VarDecCodeGen(gVar, vt);
    else
    {
        gVar->setInitializer(vt->getDefaultConstant());
    }
    cg->GlobalVarTable.addElement(Name, gVar);
}

void LocalVariableDeclaration::codegen()
{
    string Name = var->getName();
    ::Type *vt = var->getType();
    llvm::Value *val;
    AllocaInst *alloca;
    alloca = vt->allocateLLVMVariable(Name);
    if (vt->isArray())
    {
        if (exp)
        {
            static_cast<ArrayVal *>(exp.get())->gen(alloca);
        }
    }
    else if (exp)
        val = exp->codeGen();
    else
        val = vt->getDefaultConstant();
    cg->LocalVarTable.addElement(Name, alloca);
    if (!vt->isArray())
        vt->createWrite(0, val, alloca);
}

void CompoundStatement::codegen()
{
    for (auto stat = Statements.begin(); stat != Statements.end(); stat++)
    {
        stat->get()->codegen();
        if (stat->get()->isReturnStatement())
            break;
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
        var->getType()->createWrite(var->getElement(), val, globalDest);
    }
    else
    {
        var->getType()->createWrite(var->getElement(), val, dest);
    }
}

void Return::codegen()
{
    if (exp)
    {
        llvm::Value *v = exp->codeGen();
        if (!v)
            return;
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
    BasicBlock *ThenBB = BasicBlock::Create(*(cg->context), "then", func);
    BasicBlock *ElseBB = BasicBlock::Create(*(cg->context), "else", func);
    BasicBlock *AfterIfElse = BasicBlock::Create(*(cg->context), "afterif", func);
    cg->builder->CreateCondBr(cmp, ThenBB, ElseBB);
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

void BinaryExpression::VarDecCodeGen(GlobalVariable *gVar, ::Type *vt)
{
    gVar->setInitializer(vt->getDefaultConstant());
    cg->builder->SetInsertPoint(cg->getCOPBB());
    llvm::Value *v = codeGen();
    cg->builder->CreateAlignedStore(v, gVar, MaybeAlign(4));
}

llvm::Value *BinaryExpression::codeGen()
{
    llvm::Value *lhs = LVAL->codeGen();
    llvm::Value *rhs = RVAL->codeGen();
    return op->codeGen(lhs, rhs);
}

void FunctionSignature::codegen()
{
    FunctionType *funcType;
    vector<llvm::Type *> Args;
    for (auto i = args.begin(); i != args.end(); i++)
    {
        ::Type *t = i->get()->getType();
        Args.push_back(t->getLLVMType());
    }
    funcType = FunctionType::get(getRetType()->getLLVMType(), Args, false);
    Function *function = Function::Create(funcType, Function::ExternalLinkage, getName(), *(cg->module));
}

void FunctionDefinition::codeGen()
{
    cg->generatingFunctionOn();
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
        ::Type *t = n->get()->getType();
        AllocaInst *alloca;
        alloca = t->allocateLLVMVariable(name);
        cg->builder->CreateAlignedStore(ai, alloca, MaybeAlign(4));
        cg->LocalVarTable.addElement(name, alloca);
        ai->setName(name);
    }
    compoundStatements->codegen();
    cg->generatingFunctionOff();
    cg->LocalVarTable.clearTable();
}