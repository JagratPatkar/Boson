#include "ast.h"
#include "../codegen/codegen.h"
#include <vector>
#include "llvm/IR/Verifier.h"
using namespace std;
using namespace llvm;
using namespace AST;


static CodeGen* cg = CodeGen::GetInstance();


llvm::Value* ArrayVal::codeGen(){
    vector<Constant*> vals;
    for(auto i = ofVals.begin(); i != ofVals.end(); i++){
        vals.push_back(dyn_cast<Constant>(i->get()->codeGen());
    }
    return ConstantArray::get(dyn_cast<ArrayType>(type->getLLVMType()),vals);
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

void FunctionCall::VarDecCodeGen(GlobalVariable *gVar, ::Type*)
{
    cg->builder->SetInsertPoint(cg->getCOPBB());
    GlobalVariable *gvar = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = codeGen();
    cg->builder->CreateAlignedStore(v, gVar, MaybeAlign(4));
}

llvm::Value *Variable::codeGen()
{
    if (cg->getGeneratingFunction())
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

void Variable::VarDecCodeGen(GlobalVariable *gVar, ::Type* t)
{
    cg->builder->SetInsertPoint(cg->getCOPBB());
    GlobalVariable *gvar = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = cg->builder->CreateAlignedLoad(gvar,t->getAllignment() , Name.c_str());
    cg->builder->CreateAlignedStore(v, gVar, t->getAllignment());
}

void AST::Value::VarDecCodeGen(GlobalVariable *gVar, ::Type*)
{
    Constant *constant = dyn_cast<Constant>(codeGen());
    gVar->setInitializer(constant);
}

void GlobalVariableDeclaration::codegen()
{
    string Name = var->getName();
    ::Type* vt = var->getType();
    cg->module->getOrInsertGlobal(Name, vt->getLLVMType());
    GlobalVariable *gVar = cg->module->getNamedGlobal(Name);
    gVar->setAlignment(vt->getAllignment());
    if (exp) exp->VarDecCodeGen(gVar, vt);
    else{ gVar->setInitializer(vt->getDefaultConstant()); }
    cg->GlobalVarTable.addElement(Name,gVar);
}

void LocalVariableDeclaration::codegen()
{
    string Name = var->getName();
    ::Type* vt = var->getType();
    llvm::Value *val;
    if (exp) val = exp->codeGen();
    else val = vt->getDefaultConstant(); 
    AllocaInst *alloca;
    alloca = vt->allocateLLVMVariable(Name);
    cg->LocalVarTable.addElement(Name,alloca);
    cg->builder->CreateAlignedStore(val, alloca, MaybeAlign(4));
}

void CompoundStatement::codegen()
{
    for (auto stat = Statements.begin(); stat != Statements.end(); stat++)
    {
        stat->get()->codegen();
        if(stat->get()->isReturnStatement()) break;
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
        cg->builder->CreateRet(v);   
    }
    else {cg->builder->CreateRetVoid();}
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

void BinaryExpression::VarDecCodeGen(GlobalVariable *gVar, ::Type* vt)
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
    return op->codeGen(lhs,rhs);
}

void FunctionSignature::codegen()
{
    FunctionType *funcType;
    vector<llvm::Type *> Args;
    for (auto i = args.begin(); i != args.end(); i++)
    {
        ::Type* t = i->get()->getType();
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
        ::Type* t = n->get()->getType();
        AllocaInst *alloca;
        alloca = t->allocateLLVMVariable(name);
        cg->builder->CreateAlignedStore(ai, alloca, MaybeAlign(4));
        cg->LocalVarTable.addElement(name, alloca);
        ai->setName(name);
    }
    compoundStatements->codegen();
    cg->generatingFunctionOff();
    cg->LocalVarTable.clearTable();
    if (verifyFunction(*function))
    {
        string str = functionSignature->getName();
        printf("Error in %s \n", str.c_str());
    }
}