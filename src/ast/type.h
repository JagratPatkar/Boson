#include "../codegen/codegen.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
using namespace llvm;
using namespace std;

#ifndef TYPE_SYS
#define TYPE_SYS

class Type
{
protected:
    CodeGen *cg;
public:
    Type(){ cg = CodeGen::GetInstance(); }
    virtual llvm::Type *getLLVMType() = 0;
    virtual ~Type() { }
    virtual bool isInt() { return false; }
    virtual bool isDouble() { return false; }
    virtual bool isVoid() {return false;}
    virtual bool isBool() { return false; }
    virtual unique_ptr<::Type> getNew() = 0;
    virtual llvm::Constant* getDefaultConstant() = 0;
    virtual bool doesMatch(Type*) = 0;
    virtual llvm::AllocaInst* allocateLLVMVariable(const string& Name) = 0;
};


class Void : public ::Type{
    public:
    Void() {}
    llvm::Type* getLLVMType() override { return llvm::Type::getVoidTy(*(cg->context)); }
    bool isVoid() override {return true;}
    bool doesMatch(Type* t) override { return t->isVoid(); }
    unique_ptr<::Type> getNew() override { return make_unique<Void>(); }
    llvm::Constant* getDefaultConstant() override { return nullptr; }
    llvm::AllocaInst* allocateLLVMVariable(const string& Name) override { return nullptr; }
};

class Int : public ::Type
{
public:
    Int() {}
    llvm::Type* getLLVMType() override { return llvm::Type::getInt32Ty(*(cg->context)); }
    bool isInt() override { return true; }
    bool doesMatch(Type* t) override { return t->isInt(); }
    unique_ptr<::Type> getNew() override { return make_unique<Int>(); }
    llvm::Constant* getDefaultConstant() override { return ConstantInt::get(getLLVMType(),0,true); }
    llvm::AllocaInst* allocateLLVMVariable(const string& Name) override { return new AllocaInst(getLLVMType(), 0, 0, Align(4), Name.c_str(), cg->builder->GetInsertBlock()); }
};

class Double : public ::Type
{
public:
    Double() {}
    llvm::Type *getLLVMType() override { return llvm::Type::getDoubleTy(*(cg->context)); }
    bool isDouble() override { return true; }
    bool doesMatch(Type* t) override { return t->isDouble(); }
    unique_ptr<::Type> getNew() override { return make_unique<Double>(); }
    llvm::Constant* getDefaultConstant() override { return ConstantFP::get(getLLVMType(),0.0); }
    llvm::AllocaInst* allocateLLVMVariable(const string& Name) override {  return cg->builder->CreateAlloca(getLLVMType(), 0, Name.c_str()); }
};


class Bool : public ::Type{
    public:
    llvm::Type *getLLVMType() override { return llvm::Type::getInt1Ty(*(cg->context)); }
    bool isBool() override { return true; }
    bool doesMatch(Type* t) override { return t->isBool(); }
    unique_ptr<::Type> getNew() override { return make_unique<Bool>(); }
    llvm::Constant* getDefaultConstant() override { return ConstantInt::getFalse(getLLVMType()); }
    llvm::AllocaInst* allocateLLVMVariable(const string& Name) override {  return new AllocaInst(getLLVMType(), 0, 0, Align(4), Name.c_str(), cg->builder->GetInsertBlock()); }
};

#endif