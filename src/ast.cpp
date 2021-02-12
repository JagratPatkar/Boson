#include "ast.h"
#include <vector>
using namespace std;
using namespace llvm;

using namespace AST;

unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
unique_ptr<Module> module = make_unique<Module>("quark",*context);
unique_ptr<IRBuilder<>> builder = make_unique<IRBuilder<>>(*context);
map<std::string,GlobalVariable*> SymTab;
map<std::string,AllocaInst*> SymTabLoc;
FunctionType *funcType;
Function *bin_func;
BasicBlock *bb;
static bool generatingFunction = false;


void initialize(){
    funcType = FunctionType::get(builder->getVoidTy(),false);
    bin_func = Function::Create(funcType,Function::InternalLinkage,"op_func",*module); 
    bb = BasicBlock::Create(*context,"entry",bin_func);
    builder->CreateRetVoid();
}

Types AST::TypesOnToken(int type){
        if(type == -4) return  type_int;
        if(type == -5) return type_double;
        if(type == -15)return type_void;
        return type_err;
}

const char* AST::TypesName(int t){
    if(t == -1) return "int";
    if(t == -2) return "double";
    if(t == -3) return "void";
    return "unknown type";
}



llvm::Value* IntNum::codeGen(){
    Type *type = IntegerType::getInt32Ty(*context);
    return ConstantInt::get(type,Number,true);
}
llvm::Value* DoubleNum::codeGen(){
    return ConstantFP::get(*context,APFloat(Number));
}

llvm::Value* FunctionCall::codeGen(){
    Function* func = module->getFunction(Name);
    vector<llvm::Value*> ArgsValue;
    for(auto i = args.begin(); i!=args.end(); i++){
        ArgsValue.push_back(i->get()->codeGen());
        if(!ArgsValue.back()) return nullptr;
    }
    return builder->CreateCall(func,ArgsValue,"callres");
}

void FunctionCall::VarDecCodeGen(GlobalVariable* gVar,Types){
    builder->SetInsertPoint(bb);
    GlobalVariable* gvar = SymTab[Name];
    llvm::Value* v = codeGen();
    builder->CreateAlignedStore(v,gVar,MaybeAlign(4));
}

llvm::Value* Variable::codeGen(){
    if(generatingFunction){
        llvm::Value* v = SymTabLoc[Name];
        if(v) return builder->CreateAlignedLoad(v,MaybeAlign(4),Name.c_str());
    }
    GlobalVariable* gVar = SymTab[Name];
    if(!gVar) return nullptr;
    return builder->CreateAlignedLoad(gVar,MaybeAlign(4),Name.c_str());
}

void Variable::VarDecCodeGen(GlobalVariable* gVar,Types){
    builder->SetInsertPoint(bb);
    GlobalVariable* gvar = SymTab[Name];
    llvm::Value* v = builder->CreateAlignedLoad(gvar,MaybeAlign(4),Name.c_str());
    builder->CreateAlignedStore(v,gVar,MaybeAlign(4));
}

void AST::Value::VarDecCodeGen(GlobalVariable* gVar,Types){
    Constant* constant = dyn_cast<Constant>(codeGen());
    gVar->setInitializer(constant);
}

void GlobalVariableDeclaration::codegen(){
    string Name = var->getName();
    Types vt = var->getType();
    if(vt == type_int) {  module->getOrInsertGlobal(Name,Type::getInt32Ty(*context)); } 
    else module->getOrInsertGlobal(Name,Type::getDoubleTy(*context));
    GlobalVariable* gVar = module->getNamedGlobal(Name);
    gVar->setAlignment(MaybeAlign(4));
    if(exp) exp->VarDecCodeGen(gVar,vt);  
    else {
         if(vt == type_int) gVar->setInitializer(ConstantInt::get(Type::getInt32Ty(*context),0,true));
         else  gVar->setInitializer(ConstantFP::get(*context,APFloat(0.0)));
    }
    SymTab[Name] = gVar;
}


void LocalVariableDeclaration::codegen(){
    string Name = var->getName();
    Types vt = var->getType();
   
    llvm::Value* val;
    if(exp){
        val = exp->codeGen();
    }
    else{ 
        if(vt == type_int) val =  ConstantInt::get(IntegerType::getInt32Ty(*context),0,true);  
        else val = ConstantFP::get(*context,APFloat(0.0));
    }
    AllocaInst* alloca;
    if(vt == type_int) alloca = new AllocaInst(IntegerType::getInt32Ty(*context),0,0,Align(4),Name.c_str(),builder->GetInsertBlock());
    else if(vt == type_double) alloca = builder->CreateAlloca(builder->getDoubleTy(),0,Name.c_str());
    SymTabLoc[Name] = alloca;
    builder->CreateAlignedStore(val,alloca,MaybeAlign(4));
   
}

void CompoundStatement::codegen(){
    for(auto stat = Statements.begin(); stat!=Statements.end(); stat++){
        stat->get()->codegen();
    }
}


void VariableAssignment::codegen(){
    string Name = var->getName();
    llvm::Value* val = exp->codeGen();
    if(!val) return;
    llvm::Value* dest = SymTabLoc[Name];
    if(!dest){
         GlobalVariable* globalDest = SymTab[Name];
         if(!globalDest) return;
         builder->CreateAlignedStore(val,globalDest,MaybeAlign(4));
    }else{
        builder->CreateAlignedStore(val,dest,MaybeAlign(4));
    }
}


void Return::codegen(){
    if(exp){
        llvm::Value* v = exp->codeGen();
        if(!v) return;
        Types t = exp->getType();
        builder->CreateRet(v);
    }else{ builder->CreateRetVoid(); }
}

void FunctionCall::codegen(){
    Function* func = module->getFunction(Name);
    vector<llvm::Value*> ArgsValue;
    for(auto i = args.begin(); i!=args.end(); i++){
        ArgsValue.push_back(i->get()->codeGen());
        if(!ArgsValue.back()) return ;
    }
    builder->CreateCall(func,ArgsValue);
}


void IfElseStatement::codegen(){
    BasicBlock* bb =  builder->GetInsertBlock();
    Function* func =  bb->getParent();
    llvm::Value* cmp = Condition->codeGen();
    llvm::Value* cond = builder->CreateFCmpUNE(cmp,ConstantFP::get(*context,APFloat(0.0)),"ifcond");
    BasicBlock* ThenBB = BasicBlock::Create(*context,"then",func);
    BasicBlock* ElseBB =  BasicBlock::Create(*context,"else",func);
    BasicBlock* AfterIfElse = BasicBlock::Create(*context,"afterif",func);
    builder->CreateCondBr(cond,ThenBB,ElseBB);
    builder->SetInsertPoint(ThenBB);
    compoundStatements->codegen();
    builder->CreateBr(AfterIfElse);
    builder->SetInsertPoint(ElseBB);
    elseCompoundStatements->codegen();
    builder->CreateBr(AfterIfElse);
    builder->SetInsertPoint(AfterIfElse);
}

void ForStatement::codegen(){

}

void BinaryExpression::VarDecCodeGen(GlobalVariable* gVar,Types vt){
    if(vt == type_int) gVar->setInitializer(ConstantInt::get(Type::getInt32Ty(*context),0,true));
    else  gVar->setInitializer(ConstantFP::get(*context,APFloat(0.0)));
    builder->SetInsertPoint(bb);
    llvm::Value* v = codeGen();
    builder->CreateAlignedStore(v,gVar,MaybeAlign(4));
}

llvm::Value* BinaryExpression::codeGen(){
    llvm::Value* lhs = LVAL->codeGen();
    llvm::Value* rhs = RVAL->codeGen();

    if(LVAL->getType() == type_int){
        switch(op){
            case op_add : return builder->CreateAdd(lhs,rhs,"additmp");
            case op_sub : return builder->CreateSub(lhs,rhs,"subitmp");
            case op_mul : return builder->CreateMul(lhs,rhs,"mulitmp");
            case op_div : return builder->CreateSDiv(lhs,rhs,"divitmp");
            case op_less_than : 
                                lhs = builder->CreateICmpULT(lhs,rhs,"ltcmpi");
                                return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
            case op_greater_than : 
                                lhs =  builder->CreateICmpUGT(lhs,rhs,"gtcmpi");
                                return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
            case op_less_than_eq : 
                                lhs = builder->CreateICmpULE(lhs,rhs,"lecmpi");
                                return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
            case op_greater_than_eq : 
                                lhs =  builder->CreateICmpUGE(lhs,rhs,"gecmpi");
                                return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
            case op_equal_to :  
                                lhs =  builder->CreateICmpEQ(lhs,rhs,"eqi");
                                return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
            case non_op : return nullptr;
        }
    }

    switch(op){
        case op_add : return builder->CreateFAdd(lhs,rhs,"addftmp");
        case op_sub : return builder->CreateFSub(lhs,rhs,"subftmp");
        case op_mul : return builder->CreateFMul(lhs,rhs,"mulftmp");
        case op_div : return builder->CreateFDiv(lhs,rhs,"divftmp");
        case op_less_than : 
                            lhs = builder->CreateFCmpULT(lhs,rhs,"ltcmpd");
                            return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
        case op_greater_than :   
                            lhs = builder->CreateFCmpUGT(lhs,rhs,"gtcmpd");
                            return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
        case op_less_than_eq : 
                            lhs = builder->CreateFCmpULE(lhs,rhs,"lecmpd");
                            return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
        case op_greater_than_eq : 
                            lhs = builder->CreateFCmpUGE(lhs,rhs,"gecmpd");
                            return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
        case op_equal_to : 
                            lhs = builder->CreateFCmpUEQ(lhs,rhs,"eqcmp");
                            return builder->CreateUIToFP(lhs,Type::getDoubleTy(*context),"convtoFP");
        case non_op : return nullptr;
    }
    return nullptr;
}

void FunctionDefinition::codeGen(){
    generatingFunction = true;
    FunctionType *funcType;
    vector<Type*> Args;
    for(auto i = functionSignature->args.begin();i!=functionSignature->args.end();i++){
        Types t = i->get()->getType();
        if(t == type_int) Args.push_back(builder->getInt32Ty());
        else  Args.push_back(builder->getDoubleTy());
    }
    if(functionSignature->getRetType() == type_int) funcType = FunctionType::get(builder->getInt32Ty(),Args,false);
    else if(functionSignature->getRetType() == type_double) funcType = FunctionType::get(builder->getDoubleTy(),Args,false);
    else if(functionSignature->getRetType() == type_void) funcType = FunctionType::get(builder->getVoidTy(),Args,false);
    Function* function = Function::Create(funcType,Function::InternalLinkage,functionSignature->getName(),*module); 
    BasicBlock* bb = BasicBlock::Create(*context,"entry",function);
    builder->SetInsertPoint(bb);
    if(functionSignature->getName() == "start"){
        vector<llvm::Value*> args;
        builder->CreateCall(bin_func,args);
    }
    Function::arg_iterator ai,ae;
    auto n = functionSignature->args.begin();
    for(ai = function->arg_begin(), ae = function->arg_end(); ai != ae; ++ai, ++n){
        string name = n->get()->getName();
        Types t = n->get()->getType();
        AllocaInst* alloca;
        if(t == type_int) alloca = new AllocaInst(IntegerType::getInt32Ty(*context),0,0,Align(4),name,builder->GetInsertBlock());
        else alloca = builder->CreateAlloca(builder->getDoubleTy(),0,name);
        builder->CreateAlignedStore(ai,alloca,MaybeAlign(4));
        SymTabLoc[name] = alloca;
        ai->setName(name);
    }
    compoundStatements->codegen();
    generatingFunction = false;
    SymTabLoc.clear();
}