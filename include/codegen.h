#include <iostream>
#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "symboltable.h"
using namespace std;
using namespace llvm;

#ifndef CODEGEN
#define CODEGEN
class CodeGen
{
    static int counter;
    static CodeGen *obj;

    CodeGen()
    {
        context = make_unique<LLVMContext>();
        module = make_unique<Module>("boson", *context);
        builder = make_unique<IRBuilder<>>(*context);
        funcType = FunctionType::get(builder->getVoidTy(), false);
        bin_func = Function::Create(funcType, Function::InternalLinkage, "op_func", *module);
        bb = BasicBlock::Create(*context, "entry", bin_func);
        ofretBB = llvm::BasicBlock::Create(*(context), "retblock", bin_func);
        free_func = Function::Create(funcType, Function::InternalLinkage, "free_func", *module);
        fb = BasicBlock::Create(*context, "entry", free_func);
        generatingFunction = false;
        mallocFunction = Function::Create(
            FunctionType::get(Type::getInt8PtrTy(*context), {Type::getInt64Ty(*context)}, false),
            GlobalValue::ExternalLinkage, "myMalloc", module.get());
        std::vector<llvm::Type *> freeArgTypes = {llvm::Type::getInt8PtrTy(*context)};
        // Create the function type for `free`
        llvm::FunctionType *freeFnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), freeArgTypes, false);
        // Declare the `free` function in the module
        free = llvm::Function::Create(freeFnType, llvm::GlobalValue::ExternalLinkage, "myFree", module.get());

        llvm::FunctionType *iobFnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), {}, false);
        iob = llvm::Function::Create(iobFnType, llvm::GlobalValue::ExternalLinkage, "indexOutOfBounds", module.get());

        llvm::FunctionType *rcf = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*(context)),
            {llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(*(context)))},
            false);
        drfc = llvm::Function::Create(rcf, llvm::Function::ExternalLinkage, "decrementRefCount", module.get());
        irfc = llvm::Function::Create(rcf, llvm::Function::ExternalLinkage, "incrementRefCount", module.get());
        initVar = false;
        addrFunc = false;
        decBB = nullptr;
    }
    FunctionType *funcType;
    Function *bin_func;
    Function *iob;
    Function *free_func;
    Function *free;
    Function *drfc;
    Function *irfc;
    bool generatingFunction;
    BasicBlock *bb;
    BasicBlock *ofretBB;
    BasicBlock *fb;
    BasicBlock *decBB;
    int gcCounter;
    bool initVar;
    bool addrFunc;
public:
    Function *mallocFunction;
    unique_ptr<LLVMContext> context;
    unique_ptr<Module> module;
    unique_ptr<IRBuilder<>> builder;
    SymbolTable<std::string, GlobalVariable *> GlobalVarTable;
    SymbolTable<std::string, AllocaInst *> LocalVarTable;
    map<string, ::Type *> protoTable;
    AllocaInst *allocaret;
    BasicBlock *retBB;
    static CodeGen *GetInstance()
    {
        if (counter)
            return obj;
        else
        {
            obj = new CodeGen();
            counter++;
            return obj;
        }
    }

    llvm::GlobalVariable *createGlobalMP(llvm::Type *oft)
    {
        module->getOrInsertGlobal(std::to_string(++gcCounter), oft);
        return module->getNamedGlobal(std::to_string(gcCounter));
    }

    void getCOPBB() {
        Function *func = bb->getParent();
        llvm::BasicBlock &lastBB = func->getBasicBlockList().back();
        if (!(ofretBB == &lastBB))
        {
            builder->SetInsertPoint(&lastBB);
        }
        else
        {
            builder->SetInsertPoint(bb);
        } 
    }
    BasicBlock *getCFBB() { return fb; }
    void dumpIR() { module->dump(); }
    void terminateCOPBB()
    {
        Function *func = bb->getParent();
        llvm::BasicBlock &lastBB = func->getBasicBlockList().back();
        if (!(ofretBB == &lastBB))
        {
            builder->SetInsertPoint(&lastBB);
        }
        else
        {
            builder->SetInsertPoint(bb);
        }
        createOPRBB();
        builder->SetInsertPoint(ofretBB);
        builder->CreateRetVoid();
    }
    void terminateCFBB()
    {
        builder->SetInsertPoint(fb);
        for (const auto &entry : GlobalVarTable.Table)
        {
            string name = entry.first;
            GlobalVariable *alloca = entry.second;
            Type *type = alloca->getType();

            PointerType *pointerType = dyn_cast<PointerType>(type);
            if (pointerType && pointerType->getElementType()->isPointerTy())
            {
                Type *elementType = pointerType->getElementType();
                PointerType *elementPointerType = dyn_cast<PointerType>(elementType);
                if (elementPointerType && elementPointerType->getAddressSpace() == 0)
                {
                    bool is_i8_ptr = elementPointerType->getElementType()->isIntegerTy(8);
                    bool is_double_ptr = elementPointerType->getElementType()->isDoubleTy();
                    bool is_int_ptr = elementPointerType->getElementType()->isIntegerTy() && !is_i8_ptr; // Checking for integers that are not i8

                    if (is_i8_ptr || is_double_ptr || is_int_ptr)
                    {
                        // The type is a pointer to i8*, double*, or int* (i.e., i8**, double**, or int**)
                        // Load the value from the alloca and call decrementRefCount
                        llvm::Value *loadedValue = builder->CreateLoad(alloca);
                        if(is_int_ptr || is_double_ptr){
                            loadedValue = builder->CreateBitCast(loadedValue, llvm::Type::getInt8PtrTy(*(context)));
                        }
                        builder->CreateCall(getDRFCF(), {loadedValue});
                    }
                }
            }
        }
        builder->CreateRetVoid();
    }
    Function *getCOPFP() { return bin_func; }
    Function *getCFFP() { return free_func; }
    Function *getFF() { return free; }
    Function *getIOBF() { return iob; }
    Function *getDRFCF() { return drfc; }
    Function *getIRFCF() { return irfc; }
    bool getInitVar() { return initVar; }
    void setInitVar(bool v) { initVar = v; }

    bool getAddrFunc() { return addrFunc; }
    void setAddrFunc(bool v) { addrFunc = v; }

    void createOPRBB()
    {
        builder->CreateBr(ofretBB);
    }

    void setDECBB(BasicBlock* decBB){
        this->decBB = decBB;
    }
    void createDECBB()
    {
        builder->CreateBr(decBB);
    }

    void generatingFunctionOn() { generatingFunction = true; }
    void generatingFunctionOff() { generatingFunction = false; }
    bool getGeneratingFunction() { return generatingFunction; }
};
#endif