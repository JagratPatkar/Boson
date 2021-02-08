#include "ast.h"
#include <vector>
using namespace std;
using namespace llvm;

using namespace AST;

unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
unique_ptr<Module> module = make_unique<Module>("quark",*context);
unique_ptr<IRBuilder<>> builder = make_unique<IRBuilder<>>(*context);
map<std::string,GlobalVariable*> SymTab;
FunctionType *funcType;
Function *bin_func;
BasicBlock *bb;


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
    if(t == -2) return "void";
    return "unknown type";
}



llvm::Value* IntNum::codeGen(){
    Type *type = IntegerType::getInt32Ty(*context);
    return ConstantInt::get(type,Number,true);
}
llvm::Value* DoubleNum::codeGen(){
    return ConstantFP::get(*context,APFloat(Number));
}

llvm::Value* Variable::codeGen(){
    GlobalVariable* gVar = SymTab[Name];
    return builder->CreateAlignedLoad(gVar,MaybeAlign(4),Name.c_str());
    return nullptr;
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

void VariableDeclaration::codeGen(){
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

void CompoundStatement::codeGen(){

}


void VariableAssignment::codeGen(){

}


void Return::codeGen(){

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
            case non_op : return nullptr;
        }
    }

    switch(op){
        case op_add : return builder->CreateFAdd(lhs,rhs,"addftmp");
        case op_sub : return builder->CreateFSub(lhs,rhs,"subftmp");
        case op_mul : return builder->CreateFMul(lhs,rhs,"mulftmp");
        case op_div : return builder->CreateFDiv(lhs,rhs,"divftmp");
        case non_op : return nullptr;
    }
    return nullptr;
}