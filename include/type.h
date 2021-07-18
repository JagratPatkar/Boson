#include "codegen.h"
#include <memory>

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
    Type() { cg = CodeGen::GetInstance(); }
    virtual llvm::Type *getLLVMType() = 0;
    virtual ~Type() {}
    virtual bool isInt() { return false; }
    virtual bool isDouble() { return false; }
    virtual bool isVoid() { return false; }
    virtual bool isBool() { return false; }
    virtual bool isArray() { return false; }
    virtual unique_ptr<::Type> getNew() = 0;
    virtual MaybeAlign getAllignment() { return MaybeAlign(4); }
    virtual llvm::Constant *getDefaultConstant() = 0;
    virtual llvm::Constant *getConstant(int v) = 0;
    virtual bool doesMatch(Type *) = 0;
    virtual llvm::AllocaInst *allocateLLVMVariable(const string &) = 0;
    virtual void createWrite(llvm::Value *e, llvm::Value *v, llvm::Value *dest)
    {
        cg->builder->CreateAlignedStore(v, dest, getAllignment());
    }
    virtual llvm::Value *createLoad(llvm::Value *e, llvm::Value *v, const string &name)
    {
        return cg->builder->CreateAlignedLoad(v, getAllignment(), name);
    }
    virtual void CreateLLVMRet(llvm::Value * v)  { cg->builder->CreateRet(createLoad(0,v,"retval")); }
    virtual llvm::Value* createAdd(llvm::Value* v,int v1) = 0;
    virtual llvm::Value* createSub(llvm::Value* v,int v1) = 0;
    virtual llvm::Value* createNeg(llvm::Value* v) { return nullptr; }
    virtual llvm::Value* createNot(llvm::Value* v) { return nullptr; }
};

class Void : public ::Type
{
public:
    Void() {}
    llvm::Type *getLLVMType() override { return llvm::Type::getVoidTy(*(cg->context)); }
    bool isVoid() override { return true; }
    bool doesMatch(Type *t) override { return t->isVoid(); }
    unique_ptr<::Type> getNew() override { return make_unique<Void>(); }
    llvm::Constant *getDefaultConstant() override { return nullptr; }
    llvm::AllocaInst *allocateLLVMVariable(const string &Name) override { return nullptr; }
    llvm::Constant *getConstant(int v) override { return nullptr; }
    void CreateLLVMRet(llvm::Value * v) override { cg->builder->CreateRetVoid(); }
    llvm::Value* createAdd(llvm::Value* v,int v1) override { return nullptr; };
    llvm::Value* createSub(llvm::Value* v,int v1) override { return nullptr; };
};

class Int : public ::Type
{
public:
    Int() {}
    llvm::Type *getLLVMType() override { return llvm::Type::getInt32Ty(*(cg->context)); }
    bool isInt() override { return true; }
    bool doesMatch(Type *t) override { return t->isInt(); }
    unique_ptr<::Type> getNew() override { return make_unique<Int>(); }
    llvm::Constant *getDefaultConstant() override { return ConstantInt::get(getLLVMType(), 0, true); }
    llvm::Constant *getConstant(int v) override { return ConstantInt::get(getLLVMType(), v, true); }
    llvm::AllocaInst *allocateLLVMVariable(const string &Name) override { return new AllocaInst(getLLVMType(), 0, 0, Align(4), Name.c_str(), cg->builder->GetInsertBlock()); }
    llvm::Value* createAdd(llvm::Value* v,int v1) override { return cg->builder->CreateAdd(v, getConstant(v1), "additmp"); }
    llvm::Value* createSub(llvm::Value* v,int v1) override { return cg->builder->CreateSub(v, getConstant(v1), "subitmp"); }
    llvm::Value* createNeg(llvm::Value* v) override { return cg->builder->CreateNeg(v); }

};

class Double : public ::Type
{
public:
    Double() {}
    llvm::Type *getLLVMType() override { return llvm::Type::getDoubleTy(*(cg->context)); }
    bool isDouble() override { return true; }
    bool doesMatch(Type *t) override { return t->isDouble(); }
    unique_ptr<::Type> getNew() override { return make_unique<Double>(); }
    llvm::Constant *getDefaultConstant() override { return ConstantFP::get(getLLVMType(), 0.0); }
    llvm::Constant *getConstant(int v) override { return ConstantFP::get(getLLVMType(), v); }
    llvm::AllocaInst *allocateLLVMVariable(const string &Name) override { return cg->builder->CreateAlloca(getLLVMType(), 0, Name.c_str()); }
    llvm::Value* createAdd(llvm::Value* v,int v1) override { return cg->builder->CreateFAdd(v, getConstant(v1), "addftmp"); }
    llvm::Value* createSub(llvm::Value* v,int v1) override { return cg->builder->CreateFSub(v, getConstant(v1), "subftmp"); }
    llvm::Value* createNeg(llvm::Value* v) override { return cg->builder->CreateFNeg(v); }
};

class Bool : public ::Type
{
public:
    llvm::Type *getLLVMType() override { return llvm::Type::getInt1Ty(*(cg->context)); }
    bool isBool() override { return true; }
    bool doesMatch(Type *t) override { return t->isBool(); }
    unique_ptr<::Type> getNew() override { return make_unique<Bool>(); }
    llvm::Constant *getDefaultConstant() override { return ConstantInt::getFalse(getLLVMType()); }
    llvm::Constant *getConstant(int v) override { return nullptr; }
    llvm::AllocaInst *allocateLLVMVariable(const string &Name) override { return new AllocaInst(getLLVMType(), 0, 0, Align(4), Name.c_str(), cg->builder->GetInsertBlock()); }
    llvm::Value* createAdd(llvm::Value* v,int v1) override { return nullptr; };
    llvm::Value* createSub(llvm::Value* v,int v1) override { return nullptr; };
    llvm::Value* createNot(llvm::Value* v) override {
        return cg->builder->CreateNot(v);
    }
};

class Array : public ::Type
{
    unique_ptr<::Type> ofType;
    int num;

public:
    Array(int num, unique_ptr<::Type> ofType) : num(num), ofType(move(ofType)) {}
    bool isArray() override { return true; }
    MaybeAlign getAllignment() override { return MaybeAlign(16); }
    llvm::ArrayType *getLLVMType() override { return ArrayType::get(ofType->getLLVMType(), num); }
    llvm::Constant *getDefaultConstant() override
    {
        vector<Constant *> values;
        for (int i = 0; i < num; i++)
        {
            values.push_back(ofType->getDefaultConstant());
        }
        return ConstantArray::get(getLLVMType(), values);
    };
    ::Type *getOfType() { return ofType.get(); }
    bool doesMatch(Type *t) override
    {
        if (t->isArray())
        {
            Array *atype = static_cast<Array *>(t);
            return ofType->doesMatch(atype->getOfType());
        }
        return false;
    }
    bool doesMatchElement(Type *t)
    {
        return ofType->doesMatch(t);
    }
    llvm::AllocaInst *allocateLLVMVariable(const string &Name) override
    {

        return new AllocaInst(getLLVMType(), 0, 0, Align(16), Name.c_str(), cg->builder->GetInsertBlock());
    }
    unique_ptr<::Type> getNew() override { return make_unique<Array>(num, ofType->getNew()); }

    virtual void createWrite(llvm::Value *e, llvm::Value *v, llvm::Value *dest) override
    {
        Value *Idxs[] = {
            ConstantInt::get(llvm::Type::getInt32Ty(*(cg->context)), 0),
            e};
        llvm::Value *gep = cg->builder->CreateInBoundsGEP(dyn_cast<llvm::Type>(getLLVMType()), dest, Idxs);
        ofType->createWrite(0, v, gep);
    }

    virtual llvm::Value *createLoad(llvm::Value *e, llvm::Value *v, const string &name) override
    {
        Value *Idxs[] = {
            ConstantInt::get(llvm::Type::getInt32Ty(*(cg->context)), 0),
            e};
        llvm::Value *gep = cg->builder->CreateInBoundsGEP(dyn_cast<llvm::Type>(getLLVMType()), v, Idxs);
        return cg->builder->CreateAlignedLoad(gep, getAllignment(), name + "arrayidx");
    }
    void CreateLLVMRet(llvm::Value * v) override {  }
    llvm::Constant *getConstant(int v) override { return nullptr; }
    llvm::Value* createAdd(llvm::Value* v,int v1) override { return nullptr; };
    llvm::Value* createSub(llvm::Value* v,int v1) override { return nullptr; };
};

#endif