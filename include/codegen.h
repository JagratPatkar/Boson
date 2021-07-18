#include <iostream>
#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "symboltable.h"
using namespace std;
using namespace llvm;

#ifndef CODEGEN
#define CODEGEN
class CodeGen{
    static int counter;
    static CodeGen* obj;
    CodeGen() {
        context = make_unique<LLVMContext>();
        module = make_unique<Module>("boson", *context);
        builder = make_unique<IRBuilder<>>(*context);
        funcType = FunctionType::get(builder->getVoidTy(), false);
        bin_func = Function::Create(funcType, Function::InternalLinkage, "op_func", *module);
        bb = BasicBlock::Create(*context, "entry", bin_func);
        generatingFunction = false;
    }
    FunctionType *funcType;
    Function *bin_func;
    bool generatingFunction;
    BasicBlock *bb;
    public: 
    unique_ptr<LLVMContext> context;
    unique_ptr<Module> module;
    unique_ptr<IRBuilder<>> builder;
    SymbolTable<std::string, GlobalVariable *> GlobalVarTable;
    SymbolTable<std::string, AllocaInst *> LocalVarTable;
    AllocaInst* allocaret;   
    BasicBlock* retBB;
    static CodeGen* GetInstance(){
        if(counter) return obj;
        else {
            obj = new CodeGen();
            counter++;
            return obj;
        }
    }
    BasicBlock* getCOPBB(){ return bb; }

    void dumpIR() { module->dump(); }
    void terminateCOPBB() {
        builder->SetInsertPoint(bb);
        builder->CreateRetVoid();
    }
    Function* getCOPFP() { return bin_func; }

    void generatingFunctionOn(){ generatingFunction = true; }
    void generatingFunctionOff() { generatingFunction = false; }
    bool getGeneratingFunction() { return generatingFunction; }

};
#endif