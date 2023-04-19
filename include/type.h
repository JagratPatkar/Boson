#include "codegen.h"
#include <memory>
#include <unordered_map>

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
    virtual bool isObject() { return false; }
    virtual unique_ptr<::Type> getNew() = 0;
    virtual MaybeAlign getAllignment() { return MaybeAlign(4); }
    virtual llvm::Constant *getDefaultConstant() = 0;
    virtual llvm::Constant *getConstant(int v) = 0;
    virtual bool doesMatch(Type *) = 0;
    virtual llvm::AllocaInst *allocateLLVMVariable(const string &) = 0;
    virtual void createWrite(llvm::Value *e, llvm::Value *v, llvm::Value *dest)
    {
        cg->builder->CreateStore(v, dest);
    }
    virtual llvm::Value *createLoad(llvm::Value *e, llvm::Value *v, const string &name)
    {
        return cg->builder->CreateLoad(v, name);
    }
    virtual void CreateLLVMRet(llvm::Value *v) { cg->builder->CreateRet(createLoad(0, v, "retval")); }
    virtual llvm::Value *createAdd(llvm::Value *v, int v1) = 0;
    virtual llvm::Value *createSub(llvm::Value *v, int v1) = 0;
    virtual llvm::Value *createNeg(llvm::Value *v) { return nullptr; }
    virtual llvm::Value *createNot(llvm::Value *v) { return nullptr; }
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
    void CreateLLVMRet(llvm::Value *v) override { cg->builder->CreateRetVoid(); }
    llvm::Value *createAdd(llvm::Value *v, int v1) override { return nullptr; };
    llvm::Value *createSub(llvm::Value *v, int v1) override { return nullptr; };
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
    llvm::Value *createAdd(llvm::Value *v, int v1) override { return cg->builder->CreateAdd(v, getConstant(v1), "additmp"); }
    llvm::Value *createSub(llvm::Value *v, int v1) override { return cg->builder->CreateSub(v, getConstant(v1), "subitmp"); }
    llvm::Value *createNeg(llvm::Value *v) override { return cg->builder->CreateNeg(v); }
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
    llvm::Value *createAdd(llvm::Value *v, int v1) override { return cg->builder->CreateFAdd(v, getConstant(v1), "addftmp"); }
    llvm::Value *createSub(llvm::Value *v, int v1) override { return cg->builder->CreateFSub(v, getConstant(v1), "subftmp"); }
    llvm::Value *createNeg(llvm::Value *v) override { return cg->builder->CreateFNeg(v); }
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
    llvm::Value *createAdd(llvm::Value *v, int v1) override { return nullptr; };
    llvm::Value *createSub(llvm::Value *v, int v1) override { return nullptr; };
    llvm::Value *createNot(llvm::Value *v) override
    {
        return cg->builder->CreateNot(v);
    }
};

class Array : public ::Type
{
    unique_ptr<::Type> ofType;
    int num;
    string name;

public:
    Array(int num, unique_ptr<::Type> ofType) : num(num), ofType(move(ofType)) {}
    bool isArray() override { return true; }
    MaybeAlign getAllignment() override { return MaybeAlign(16); }

    llvm::PointerType *getLLVMType() override { return ofType->getLLVMType()->getPointerTo(); }

    llvm::Constant *getDefaultConstant() override
    {
        return llvm::Constant::getNullValue(getLLVMType());
    }

    ::Type *getOfType() { return ofType.get(); }

    int getSize()
    {
        return num;
    }

    void setName(const string &name)
    {
        this->name = name;
    }

    string getName()
    {
        return name;
    }

    bool isSameSize(int num)
    {
        return num == this->num;
    }

    bool doesMatch(Type *t) override
    {
        if (t->isArray())
        {
            Array *atype = static_cast<Array *>(t);
            bool cond = atype->isSameSize(num) && ofType->doesMatch(atype->getOfType());
            return cond;
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

    void createBoundCheck(llvm::Value *e)
    {
        llvm::Function *iob = cg->getIOBF();

        // Check if the index is negative
        llvm::Value *isNegative = cg->builder->CreateICmpSLT(e, llvm::ConstantInt::get(e->getType(), 0));

        // Check if the index is greater than or equal to the array size
        llvm::Value *isOutOfBounds = cg->builder->CreateICmpSGE(e, llvm::ConstantInt::get(e->getType(), getSize()));

        // Combine the two checks
        llvm::Value *isInvalidIndex = cg->builder->CreateOr(isNegative, isOutOfBounds);

        // Create basic blocks for the boundary check and the rest of the function
        llvm::BasicBlock *boundaryCheckBB = llvm::BasicBlock::Create(*(cg->context), "boundary_check", cg->builder->GetInsertBlock()->getParent());
        llvm::BasicBlock *continueBB = llvm::BasicBlock::Create(*(cg->context), "continue");
        // If the index is invalid, branch to the boundary check; otherwise, continue
        cg->builder->CreateCondBr(isInvalidIndex, boundaryCheckBB, continueBB);
        // Boundary check: call the 'iob' function and return
        cg->builder->SetInsertPoint(boundaryCheckBB);
        cg->builder->CreateCall(iob);
        if (cg->getGeneratingFunction())
        {
            cg->builder->CreateBr(cg->retBB);
        }
        else
        {
            cg->createOPRBB();
        }
        cg->builder->GetInsertBlock()->getParent()->getBasicBlockList().push_back(continueBB);
        cg->builder->SetInsertPoint(continueBB);
    }

    virtual void createWriteElement(llvm::Value *e, llvm::Value *v, llvm::Value *dest)
    {
        createBoundCheck(e);
        llvm::Value *arrayBase = cg->builder->CreateLoad(dest);
        Value *Idxs[] = {e};
        llvm::Value *gep = cg->builder->CreateInBoundsGEP(ofType->getLLVMType(), arrayBase, Idxs);
        ofType->createWrite(0, v, gep);
    }

    virtual llvm::Value *createLoadElement(llvm::Value *e, llvm::Value *v, const string &name)
    {
        createBoundCheck(e);
        llvm::Value *arrayBase = cg->builder->CreateLoad(v);
        Value *Idxs[] = {e};
        llvm::Value *gep = cg->builder->CreateInBoundsGEP(ofType->getLLVMType(), arrayBase, Idxs);
        return cg->builder->CreateAlignedLoad(gep, getAllignment(), name + "arrayidx");
    }

    virtual void createWrite(llvm::Value *e, llvm::Value *v, llvm::Value *dest) override
    {
        llvm::Value *oldValue = nullptr;
        llvm::Function *decrementRefCountFunc = cg->getDRFCF();

        if (!cg->getInitVar())
        {
            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(dest))
            {
                if (allocaInst->getName().str() != "ret")
                {
                    oldValue = cg->builder->CreateLoad(dest);
                }
            }
            else if (llvm::GlobalVariable *gv = llvm::dyn_cast<llvm::GlobalVariable>(dest))
            {
                oldValue = cg->builder->CreateLoad(dest);
            }

            if (oldValue)
            {
                llvm::Value *oldValue_i8 = cg->builder->CreateBitCast(oldValue, llvm::Type::getInt8PtrTy(*(cg->context)));
                cg->builder->CreateCall(decrementRefCountFunc, {oldValue_i8});
            }
        }
        if (!cg->getAddrFunc())
        {
            llvm::Function *incrementRefCountFunc = cg->getIRFCF();
            llvm::Value *v_i8 = cg->builder->CreateBitCast(v, llvm::Type::getInt8PtrTy(*(cg->context)));
            cg->builder->CreateCall(incrementRefCountFunc, {v_i8});
        }

        cg->builder->CreateStore(v, dest);
    }
    virtual llvm::Value *createLoad(llvm::Value *e, llvm::Value *v, const string &name) override
    {
        return cg->builder->CreateLoad(v, name);
    }

    llvm::Constant *getConstant(int v) override { return nullptr; }
    llvm::Value *createAdd(llvm::Value *v, int v1) override { return nullptr; };
    llvm::Value *createSub(llvm::Value *v, int v1) override { return nullptr; };
};

class ObjectTy : public ::Type
{
    std::vector<std::pair<std::string, std::unique_ptr<Type>>> properties;
    llvm::StructType *structType;
    string name;

public:
    ObjectTy()
    {
        structType = nullptr;
    }
    void setName(const string &name)
    {
        this->name = name;
    }
    string getName()
    {
        return name;
    }

    llvm::StructType *getStructType()
    {
        return structType;
    }

    void createType()
    {
        if (structType)
            return;
        std::vector<llvm::Type *> memberTypes;
        for (auto &member : properties)
        {
            memberTypes.push_back(member.second->getLLVMType());
        }
        structType = llvm::StructType::create(memberTypes, name);
    }

    llvm::Type *getLLVMType() override
    {
        return llvm::Type::getInt8PtrTy(*cg->context);
    }

    bool isObject() override { return true; }

    size_t getPropertiesSize() const
    {
        return properties.size();
    }

    Type *getPropertyTypeAt(size_t index) const
    {
        if (index < properties.size())
        {
            return properties[index].second.get();
        }
        return nullptr;
    }

    bool doesMatch(Type *t) override
    {
        if (!t->isObject())
        {
            return false;
        }
        ObjectTy *otherObj = static_cast<ObjectTy *>(t);
        if (properties.size() != otherObj->getPropertiesSize())
        {
            return false;
        }
        for (size_t i = 0; i < properties.size(); ++i)
        {
            auto &prop1 = properties[i];
            auto prop2 = otherObj->getPropertyTypeAt(i);
            if (!prop1.second->doesMatch(prop2))
            {
                return false;
            }
        }
        return true;
    }

    std::unique_ptr<Type> getNew() override
    {
        auto newObj = std::make_unique<ObjectTy>();
        newObj->setName(name);
        newObj->structType = structType;
        for (const auto &prop : properties)
        {
            newObj->properties.push_back({prop.first, prop.second->getNew()});
        }
        return newObj;
    }
    llvm::Constant *getDefaultConstant() override
    {
        return llvm::Constant::getNullValue(getLLVMType());
    }
    llvm::Constant *getConstant(int v) override { return nullptr; }

    llvm::AllocaInst *allocateLLVMVariable(const std::string &Name) override
    {
        llvm::Type *objPtrType = getLLVMType();
        llvm::AllocaInst *objPtr = cg->builder->CreateAlloca(objPtrType, nullptr, Name);
        return objPtr;
    }

    std::tuple<int, std::unique_ptr<Type>> getProperty(const std::string &name) const
    {
        for (int i = 0; i < properties.size(); ++i)
        {
            auto &prop = properties[i];
            if (prop.first == name)
            {
                return std::make_tuple(i, prop.second->getNew());
            }
        }
        return std::make_tuple(-1, nullptr);
    }

    void setProperty(std::string name, unique_ptr<Type> value)
    {
        for (auto &prop : properties)
        {
            if (prop.first == name)
            {
                prop.second = std::move(value);
                return;
            }
        }
        properties.push_back({name, std::move(value)});
    }

    llvm::Value *createAdd(llvm::Value *v, int v1) override { return nullptr; };
    llvm::Value *createSub(llvm::Value *v, int v1) override { return nullptr; };

    virtual void createWriteN(int i, llvm::Value *v, llvm::Value *dest)
    {
        llvm::Value *p = cg->builder->CreateLoad(dest, "pr");
        llvm::Value *bc = cg->builder->CreateBitCast(p, getStructType()->getPointerTo());
        llvm::Value *memberPointer = cg->builder->CreateStructGEP(structType, bc, i, properties[i].first + "Pointer");
        cg->builder->CreateStore(v, memberPointer);
    }

    virtual llvm::Value *createLoadN(int i, llvm::Value *v, const string &name)
    {
        llvm::Value *p = cg->builder->CreateLoad(v, "pr");
        llvm::Value *bc = cg->builder->CreateBitCast(p, getStructType()->getPointerTo());
        llvm::Value *memberPtr = cg->builder->CreateStructGEP(structType, bc, i, properties[i].first + "Pointer");
        return cg->builder->CreateLoad(memberPtr, name);
    }

    virtual void createWrite(llvm::Value *e, llvm::Value *v, llvm::Value *dest) override
    {
        llvm::Value *oldValue = nullptr;
        llvm::Function *decrementRefCountFunc = cg->getDRFCF();
        if (!cg->getInitVar())
        {
            if (llvm::AllocaInst *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(dest))
            {
                if (allocaInst->getName().str() != "ret")
                {
                    oldValue = cg->builder->CreateLoad(dest);
                }
            }
            else if (llvm::GlobalVariable *gv = llvm::dyn_cast<llvm::GlobalVariable>(dest))
            {
                oldValue = cg->builder->CreateLoad(dest);
            }

            if (oldValue)
            {
                cg->builder->CreateCall(decrementRefCountFunc, {oldValue});
            }
        }
        if (!cg->getAddrFunc())
        {
            llvm::Function *incrementRefCountFunc = cg->getIRFCF();
            cg->builder->CreateCall(incrementRefCountFunc, {v});
        }
        cg->builder->CreateStore(v, dest);
    }
    virtual llvm::Value *createLoad(llvm::Value *e, llvm::Value *v, const string &name) override
    {
        return cg->builder->CreateLoad(v, name);
    }
};

#endif