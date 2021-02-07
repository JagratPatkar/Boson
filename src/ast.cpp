#include "ast.h"
#include <vector>
using namespace std;
using namespace llvm;

using namespace AST;

unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
unique_ptr<Module> module = make_unique<Module>("quark",*context);
unique_ptr<IRBuilder<>> builder = make_unique<IRBuilder<>>(*context);
map<std::string,GlobalVariable*> SymTab;
vector<Function*> bin_op_funcs;

Types AST::TypesOnToken(int type){
        if(type == -4) return  type_int;
        if(type == -5) return type_double;
        return type_void;
}

const char* AST::TypesName(int t){
    if(t == -1) return "int";
    if(t == -2) return "double";
    return "void";
}



llvm::Value* IntNum::codeGen(){
    Type *type = IntegerType::getInt32Ty(*context);
    return ConstantInt::get(type,Number,true);
}
llvm::Value* DoubleNum::codeGen(){
    return ConstantFP::get(*context,APFloat(Number));
}

llvm::Value* Variable::codeGen(){
    // Value* v = SymTab[Name];
    // return builder->CreateLoad(v,Name.c_str());
    return nullptr;
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

void CompundStatement::codeGen(){

}


void VariableAssignment::codeGen(){

}

void BinaryExpression::VarDecCodeGen(GlobalVariable* gVar,Types vt){
    fprintf(stderr,"here");
    if(vt == type_int) gVar->setInitializer(ConstantInt::get(Type::getInt32Ty(*context),0,true));
    else  gVar->setInitializer(ConstantFP::get(*context,APFloat(0.0)));
    FunctionType *funcType = FunctionType::get(builder->getVoidTy(),false);
    Function *bin_func = Function::Create(funcType,Function::InternalLinkage,"bin_func",*module); 
    BasicBlock *bb = BasicBlock::Create(*context,"entry",bin_func);
    builder->SetInsertPoint(bb);
    llvm::Value* v = codeGen();
    builder->CreateAlignedStore(v,gVar,MaybeAlign(4));
    builder->CreateRetVoid();
    bin_op_funcs.push_back(bin_func);
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
        case op_add : return builder->CreateFAdd(lhs,rhs,"additmp");
        case op_sub : return builder->CreateFSub(lhs,rhs,"subitmp");
        case op_mul : return builder->CreateFMul(lhs,rhs,"mulitmp");
        case op_div : return builder->CreateFDiv(lhs,rhs,"divitmp");
        case non_op : return nullptr;
    }
    return nullptr;
}