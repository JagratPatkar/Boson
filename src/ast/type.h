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
    virtual llvm::Type *getLLVMType() = 0;
    virtual ~Type() { }
    virtual bool isInt() { return false; }
    virtual bool isDouble() { return false; }
    virtual bool isVoid() {return false;}
    virtual bool doesMatch(Type*) = 0;
};


class Void : public ::Type{
    public:
    Void() {}
    llvm::Type* getLLVMType() override { return llvm::Type::getVoidTy(*(cg->context)); }
    bool isVoid() override {return true;}
    bool doesMatch(Type* t) override { return t->isVoid(); }
};

class Int : public ::Type
{
public:
    Int() {}
    llvm::Type* getLLVMType() override { return llvm::Type::getInt32Ty(*(cg->context)); }
    bool isInt() override { return true; }
    bool doesMatch(Type* t) override { return t->isInt(); }
};

class Double : public ::Type
{
public:
    Double() {}
    llvm::Type *getLLVMType() override { return llvm::Type::getDoubleTy(*(cg->context)); }
    bool isDouble() override { return true; }
    bool doesMatch(Type* t) override { return t->isDouble(); }
};

#endif