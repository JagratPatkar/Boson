#include "ast.h"
using namespace llvm;

using namespace AST;

unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
unique_ptr<Module> module = make_unique<Module>("quark",*context);

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