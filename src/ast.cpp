#include "ast.h"
#include "codegen.h"
#include <vector>
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"

using namespace std;
using namespace llvm;
using namespace AST;

static CodeGen *cg = CodeGen::GetInstance();

llvm::Value *ObjMember::codeGen()
{
    // Generate the code for the object member value
    return value->codeGen();
}

llvm::Value *ObjLiteral::codeGen()
{
    return nullptr;
}

void ObjLiteral::gen(llvm::Value *a, ::Type *t)
{
    if (isExp())
    {
        llvm::Value *ev = exp->codeGen();
        llvm::Function *incrementRefCountFunc = cg->getIRFCF();
        if (exp->isVariable())
        {
            cg->builder->CreateCall(incrementRefCountFunc, {ev});
        }
        cg->builder->CreateStore(ev, a);
        return;
    }
    ObjectTy *pt = dynamic_cast<ObjectTy *>(t);
    llvm::StructType *objStructType = pt->getStructType();
    string name = pt->getName();

    uint64_t sizeInBits = cg->module->getDataLayout().getTypeSizeInBits(objStructType);
    uint64_t sizeInBytes = (sizeInBits + 7) / 8;
    llvm::Value *objMallocSize = llvm::ConstantInt::get(*cg->context, llvm::APInt(64, sizeInBytes));
    llvm::Value *objMemory = cg->builder->CreateCall(cg->mallocFunction, objMallocSize, name + "Memory");
    cg->builder->CreateStore(objMemory, a);
    llvm::Value *objStructPointer = cg->builder->CreateBitCast(objMemory, objStructType->getPointerTo(), name + "Pointer");

    
    for (size_t i = 0; i < members.size(); ++i)
    {
        llvm::Value *memberValue;
        if (!members[i]->value)
        {
            auto md = static_cast<ObjectTy *>(t)->getProperty(members[i]->name);
            memberValue = std::get<1>(md)->getDefaultConstant();
        }
        else
        {
            memberValue = members[i]->value->codeGen();
        }
        llvm::Value *memberPointer = cg->builder->CreateStructGEP(objStructType, objStructPointer, i, members[i]->name + "Pointer");
        cg->builder->CreateStore(memberValue, memberPointer);
    }
}

void ObjLiteral::VarDecCodeGen(GlobalVariable *gVar, ::Type *type)
{
    cg->getCOPBB();
    gen(gVar, type);
    gVar->setInitializer(type->getDefaultConstant());
}

llvm::Value *ArrayVal::codeGen()
{
    return nullptr;
}

llvm::Value *Neg::codeGen(llvm::Value *dest, Expression *e)
{
    return op_type->createNeg(dest);
}

llvm::Value *UnaryExpression::codeGen()
{
    llvm::Value *v;
    if (VAL->isVariable())
    {
        Variable *var = static_cast<Variable *>(VAL.get());
        if (cg->getGeneratingFunction() && cg->LocalVarTable.doesElementExist(var->getName()))
        {
            auto v = cg->LocalVarTable.getElement(var->getName());
            return op->codeGen(v, VAL.get());
        }
        else if (cg->GlobalVarTable.doesElementExist(var->getName()))
        {
            auto v = cg->GlobalVarTable.getElement(var->getName());
            return op->codeGen(v, VAL.get());
        }
        return nullptr;
    }

    return op->codeGen(VAL->codeGen(), VAL.get());
}

llvm::Value *AddPostIncrement::codeGen(llvm::Value *dest, Expression *e)
{
    auto var = static_cast<Variable *>(e);
    llvm::Value *v1 = e->codeGen();
    llvm::Value *rt = op_type->createAdd(v1, 1);
    if (var->getObjectElemFlag())
    {
        auto objptr = static_cast<ObjectTy *>(var->getObjectType());
        objptr->createWriteN(var->getElementNumber(), rt, dest);
    }
    else if (var->getArrayFlag())
    {
        auto arryptr = static_cast<Array *>(var->getArrayType());
        arryptr->createWriteElement(var->getElement(),
                                    rt,
                                    dest);
    }
    else
    {
        var->getType()->createWrite(var->getElement(), rt, dest);
    }
    return v1;
}

llvm::Value *SubPostIncrement::codeGen(llvm::Value *dest, Expression *e)
{
    llvm::Value *v1 = e->codeGen();
    auto var = static_cast<Variable *>(e);
    llvm::Value *rt = op_type->createSub(v1, 1);
    if (var->getObjectElemFlag())
    {
        auto objptr = static_cast<ObjectTy *>(var->getObjectType());
        objptr->createWriteN(var->getElementNumber(), rt, dest);
    }
    else if (var->getArrayFlag())
    {
        auto arryptr = static_cast<Array *>(var->getArrayType());
        arryptr->createWriteElement(var->getElement(),
                                    rt,
                                    dest);
    }
    else
    {
        var->getType()->createWrite(var->getElement(), rt, dest);
    }
    return v1;
}

llvm::Value *AddPreIncrement::codeGen(llvm::Value *dest, Expression *e)
{
    auto var = static_cast<Variable *>(e);
    llvm::Value *v1 = e->codeGen();
    llvm::Value *rt = op_type->createAdd(v1, 1);
    if (var->getObjectElemFlag())
    {
        auto objptr = static_cast<ObjectTy *>(var->getObjectType());
        objptr->createWriteN(var->getElementNumber(), rt, dest);
    }
    else if (var->getArrayFlag())
    {
        auto arryptr = static_cast<Array *>(var->getArrayType());
        arryptr->createWriteElement(var->getElement(),
                                    rt,
                                    dest);
    }
    else
    {
        var->getType()->createWrite(var->getElement(), rt, dest);
    }
    return e->codeGen();
}

llvm::Value *SubPreIncrement::codeGen(llvm::Value *dest, Expression *e)
{
    auto var = static_cast<Variable *>(e);
    llvm::Value *v1 = e->codeGen();
    llvm::Value *rt = op_type->createSub(v1, 1);
    if (var->getObjectElemFlag())
    {
        auto objptr = static_cast<ObjectTy *>(var->getObjectType());
        objptr->createWriteN(var->getElementNumber(), rt, dest);
    }
    else if (var->getArrayFlag())
    {
        auto arryptr = static_cast<Array *>(var->getArrayType());
        arryptr->createWriteElement(var->getElement(),
                                    rt,
                                    dest);
    }
    else
    {
        var->getType()->createWrite(var->getElement(), rt, dest);
    }
    return e->codeGen();
}

void UnaryExpression::VarDecCodeGen(GlobalVariable *gVar, ::Type *t)
{
    cg->getCOPBB();
    codeGen();
}

void ArrayVal::gen(llvm::Value *vt)
{
    if (isExp())
    {
        llvm::Value *ev = exp->codeGen();
        llvm::Function *incrementRefCountFunc = cg->getIRFCF();
        if (exp->isVariable())
        {
            cg->builder->CreateCall(incrementRefCountFunc, {ev});
        }
        cg->builder->CreateStore(ev, vt);
        return;
    }
    Array *t = static_cast<Array *>(getType());
    auto et = t->getOfType();
    llvm::Value *arraySize = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*cg->context), t->getSize() * et->getLLVMType()->getScalarSizeInBits() / 8);
    llvm::Value *arrayMemory = cg->builder->CreateCall(cg->mallocFunction, arraySize, name + "Memory");
    llvm::Value *arrayPointer = cg->builder->CreateBitCast(arrayMemory, t->getLLVMType(), name);
    cg->builder->CreateStore(arrayPointer, vt);



    for (size_t i = 0; i < t->getSize(); ++i)
    {
        llvm::Value *v;
        if (!ofVals.empty() && ofVals[i])
        {
            v = ofVals[i].get()->codeGen();
        }
        else
        {
            v = et->getDefaultConstant();
        }
        t->createWriteElement(ConstantInt::get(llvm::Type::getInt32Ty(*(cg->context)), i), v, vt);
    }
}

void ArrayVal::VarDecCodeGen(GlobalVariable *gVar, ::Type *t)
{
    cg->getCOPBB();
    gen(gVar);
    gVar->setInitializer(type->getDefaultConstant());
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
    cg->getCOPBB();
    GlobalVariable *gvar = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = codeGen();
    cg->builder->CreateAlignedStore(v, gVar, t->getAllignment());
}

llvm::Value *Variable::codeGen()
{
    if (cg->getGeneratingFunction() && cg->LocalVarTable.doesElementExist(Name))
    {
        llvm::Value *v = cg->LocalVarTable.getElement(Name);
        if (isArrayElem)
        {
            Array *pt = dynamic_cast<Array *>(arrayType.get());
            return pt->createLoadElement(getElement(),
                                         v,
                                         Name);
        }
        if (isObjectElem)
        {

            ObjectTy *pt = dynamic_cast<ObjectTy *>(objectType.get());
            return pt->createLoadN(getElementNumber(), v, Name);
        }

        return type->createLoad(0, v, Name);
    }
    else if (cg->GlobalVarTable.doesElementExist(Name))
    {
        GlobalVariable *gVar = cg->GlobalVarTable.getElement(Name);
        if (isArrayElem)
        {
            Array *pt = dynamic_cast<Array *>(arrayType.get());
            return pt->createLoadElement(getElement(),
                                         gVar,
                                         Name);
        }
        if (isObjectElem)
        {
            ObjectTy *pt = dynamic_cast<ObjectTy *>(objectType.get());
            return pt->createLoadN(getElementNumber(), gVar, Name);
        }
        return type->createLoad(0, gVar, Name);
    }
    return nullptr;
}

void Variable::VarDecCodeGen(GlobalVariable *gVar, ::Type *t)
{
    cg->getCOPBB();
    GlobalVariable *lg = cg->GlobalVarTable.getElement(Name);
    llvm::Value *v = cg->builder->CreateAlignedLoad(lg, t->getAllignment(), Name.c_str());
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
    gVar->setInitializer(vt->getDefaultConstant());
    if (exp)
        exp->VarDecCodeGen(gVar, vt);
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
    else if (vt->isObject())
    {
        if (exp)
        {
            static_cast<ObjLiteral *>(exp.get())->gen(alloca, vt);
        }
    }
    else if (exp)
        val = exp->codeGen();
    else
        val = vt->getDefaultConstant();
    cg->LocalVarTable.addElement(Name, alloca);
    if (!vt->isArray() && !vt->isObject())
    {
        vt->createWrite(0, val, alloca);
    }
}

void CompoundStatement::codegen()
{
    for (auto stat = Statements.begin(); stat != Statements.end(); stat++)
    {
        stat->get()->codegen();
        if (stat->get()->isReturnStatement())
        {
            setHasRetTrue();
            break;
        }
    }
}

void VariableAssignment::codegen()
{
    if (getGlobally())
        cg->getCOPBB();
    string Name = var->getName();
    llvm::Value *val = exp->codeGen();
    if (!val)
        return;
    if (cg->LocalVarTable.doesElementExist(Name))
    {
        llvm::Value *dest = cg->LocalVarTable.getElement(Name);
        if (var->getObjectElemFlag())
        {
            ObjectTy *pt = dynamic_cast<ObjectTy *>(var->getObjectType());
            pt->createWriteN(var->getElementNumber(), val, dest);
        }
        else if (var->getArrayFlag())
        {
            Array *pt = dynamic_cast<Array *>(var->getArrayType());
            pt->createWriteElement(var->getElement(), val, dest);
        }
        else
        {
            if(!exp->isVariable()){
                cg->setAddrFunc(true);
            }
            var->getType()->createWrite(var->getElement(), val, dest);
            if(!exp->isVariable()){
                cg->setAddrFunc(false);
            }
        }
    }
    else if (cg->GlobalVarTable.doesElementExist(Name))
    {
        GlobalVariable *globalDest = cg->GlobalVarTable.getElement(Name);
        if (var->getObjectElemFlag())
        {
            ObjectTy *pt = dynamic_cast<ObjectTy *>(var->getObjectType());
            pt->createWriteN(var->getElementNumber(), val, globalDest);
        }
        else if (var->getArrayFlag())
        {
            Array *pt = dynamic_cast<Array *>(var->getArrayType());
            pt->createWriteElement(var->getElement(), val, globalDest);
        }
        else
        {
            if(!exp->isVariable()){
                cg->setAddrFunc(true);
            }
            var->getType()->createWrite(var->getElement(), val, globalDest);
            if(!exp->isVariable()){
                cg->setAddrFunc(false);
            }
        }
    }
}

void Return::codegen()
{
    if (exp)
    {
        llvm::Value *v = exp->codeGen();
        if (!v)
            return;
        exp->getType()->createWrite(0, v, cg->allocaret);
    }
    // cg->createDECBB();
}

void FunctionCall::codegen()
{
    if (getGlobally())
        cg->getCOPBB();
    Function *func = cg->module->getFunction(Name);
    vector<llvm::Value *> ArgsValue;
    for (auto i = args.begin(); i != args.end(); i++)
    {
        ArgsValue.push_back(i->get()->codeGen());
        if (!ArgsValue.back())
            return;
    }
    if(dalloc){
        llvm::Value* v = cg->builder->CreateCall(func, ArgsValue, "callres");
        llvm::Function *decrementRefCountFunc = cg->getDRFCF();
        cg->builder->CreateCall(decrementRefCountFunc, {v});
    }else{
        cg->builder->CreateCall(func, ArgsValue);
    }
}

void IfElseStatement::codegen()
{
    BasicBlock *bb = cg->builder->GetInsertBlock();
    Function *func = bb->getParent();
    llvm::Value *cmp = Condition->codeGen();
    BasicBlock *ThenBB = BasicBlock::Create(*(cg->context), "then", func);
    BasicBlock *ElseBB = BasicBlock::Create(*(cg->context), "else", func);
    BasicBlock *AfterIfElse = BasicBlock::Create(*(cg->context), "afterif", func);
    if (elseCompoundStatements)
        cg->builder->CreateCondBr(cmp, ThenBB, ElseBB);
    else
    {
        cg->builder->CreateCondBr(cmp, ThenBB, AfterIfElse);
        ElseBB->eraseFromParent();
    }
    cg->builder->SetInsertPoint(ThenBB);
    compoundStatements->codegen();
    if (compoundStatements->getHasRet())
        cg->createDECBB();
    else
        cg->builder->CreateBr(AfterIfElse);
    if (elseCompoundStatements)
    {
        cg->builder->SetInsertPoint(ElseBB);
        elseCompoundStatements->codegen();
        if (elseCompoundStatements->getHasRet())
            cg->createDECBB();
        else
            cg->builder->CreateBr(AfterIfElse);
    }
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


void ForEachStatement::codegen()
{
    BasicBlock *bb = cg->builder->GetInsertBlock();
    Function *func = bb->getParent();
    BasicBlock *LoopBB = BasicBlock::Create(*(cg->context), "loop", func);
    BasicBlock *AfterLoopBB = BasicBlock::Create(*(cg->context), "afterloop", func);

   
    AllocaInst *alloca;
    alloca = varType->allocateLLVMVariable(sinkName);
    cg->LocalVarTable.addElement(sinkName, alloca);    

    llvm::Value* val;
    AllocaInst *counterAlloca;
    counterAlloca = counterType->allocateLLVMVariable(sinkName + "counter");
    val = counterType->getDefaultConstant();
    counterType->createWrite(0, val, counterAlloca);

    Array* arrType = static_cast<Array*>(sourceType.get());
    llvm::Value *arraySize = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*(cg->context)), arrType->getSize(), true);
    llvm::Value* counterValue = varType->createLoad(0, counterAlloca, sinkName + "counterLoad");
    llvm::Value *condition =  cg->builder->CreateICmpSLT(counterValue, arraySize, "cmp");
  
    cg->builder->CreateCondBr(condition, LoopBB, AfterLoopBB);
    cg->builder->SetInsertPoint(LoopBB);

    llvm::Value *arrayElemVal;
    if (cg->getGeneratingFunction() && cg->LocalVarTable.doesElementExist(sourceName))
    {
        llvm::Value *v = cg->LocalVarTable.getElement(sourceName);
        llvm::Value *cv = varType->createLoad(0, counterAlloca, sinkName + "counterLoad");
        arrayElemVal = arrType->createLoadElement(cv, v, "allocaValue");
    }
    else{
        GlobalVariable *gVar = cg->GlobalVarTable.getElement(sourceName);   
        llvm::Value *cv = varType->createLoad(0, counterAlloca, sinkName + "counterLoad"); 
        arrayElemVal = arrType->createLoadElement(cv, gVar, "allocaValue");
    }

    varType->createWrite(0,arrayElemVal, alloca);

    compoundStatement->codegen();
    
    llvm::Value *cv = varType->createLoad(0, counterAlloca, sinkName + "counterLoad");
    counterType->createWrite(0,counterType->createAdd(cv, 1), counterAlloca);

    arraySize = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*(cg->context)), arrType->getSize(), true);
    counterValue = varType->createLoad(0, counterAlloca, sinkName + "counterLoad");
    condition =  cg->builder->CreateICmpSLT(counterValue, arraySize, "cmp");
  
    cg->builder->CreateCondBr(condition, LoopBB, AfterLoopBB);
    cg->builder->SetInsertPoint(AfterLoopBB);
}


void BinaryExpression::VarDecCodeGen(GlobalVariable *gVar, ::Type *vt)
{
    gVar->setInitializer(vt->getDefaultConstant());
    cg->getCOPBB();
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
    cg->allocaret = functionSignature->getRetType()->allocateLLVMVariable("ret");
    cg->retBB = BasicBlock::Create(*(cg->context), "retblock", function);
    llvm::BasicBlock *decBB = BasicBlock::Create(*(cg->context), "decblock", function);
    cg->setDECBB(decBB);
    cg->builder->SetInsertPoint(cg->retBB);
    // Call the freeMemory function if the function is start
    if (functionSignature->getName() == "start")
    {
        vector<llvm::Value *> args;
        cg->builder->CreateCall(cg->getCFFP(), args);
    }
    functionSignature->retType.get()->CreateLLVMRet(cg->allocaret);
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
        cg->setInitVar(true);
        t->createWrite(0, ai, alloca);
        cg->setInitVar(false);
        cg->LocalVarTable.addElement(name, alloca);
        ai->setName(name);
    }
    compoundStatements->codegen();
    cg->builder->CreateBr(decBB);
    cg->builder->SetInsertPoint(decBB);
    for (const auto &entry : cg->LocalVarTable.Table)
    {
        string name = entry.first;
        AllocaInst *alloca = entry.second;
        llvm::Type *type = alloca->getType();

        PointerType *pointerType = dyn_cast<PointerType>(type);
        if (pointerType && pointerType->getElementType()->isPointerTy())
        {
            llvm::Type *elementType = pointerType->getElementType();
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
                    llvm::Value *loadedValue = cg->builder->CreateLoad(alloca);
                    if(is_int_ptr || is_double_ptr){
                        loadedValue = cg->builder->CreateBitCast(loadedValue, llvm::Type::getInt8PtrTy(*(cg->context)));
                    }
                    cg->builder->CreateCall(cg->getDRFCF(), {loadedValue});
                }
            }
        }
    }
    cg->builder->CreateBr(cg->retBB);
    cg->generatingFunctionOff();
    cg->setDECBB(nullptr);
    cg->LocalVarTable.clearTable();
    cg->allocaret = nullptr;
    cg->retBB = nullptr;

    if (verifyFunction(*function))
    {
        cerr << " Error in function " << functionSignature->getName() << endl;
    }
}