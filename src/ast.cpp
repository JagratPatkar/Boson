#include "ast.h"
using namespace llvm;

using namespace AST;

unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
unique_ptr<Module> module = make_unique<Module>("quark",*context);
unique_ptr<IRBuilder<>> builder = make_unique<IRBuilder<>>(*context);
map<std::string,GlobalVariable*> SymTab;


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
    return nullptr;
}




void VariableDeclaration::codeGen(){
    string Name = var->getName();
    Types vt = var->getType();
    llvm::Value* v;
    if(exp) v = exp->codeGen();
    else v = nullptr;
    if(vt == type_int) module->getOrInsertGlobal(Name,Type::getInt32Ty(*context));
    else module->getOrInsertGlobal(Name,Type::getDoubleTy(*context));
    GlobalVariable* gVar = module->getNamedGlobal(Name);
    if(v) builder->CreateStore(v,gVar);
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