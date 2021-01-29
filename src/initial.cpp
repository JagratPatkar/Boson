#include <stdio.h>
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
using namespace std;
using namespace llvm;

int main(){
    printf("A Quark Language");
    LLVMContext& context = getGlobalContext();
    Module* module = new Module("quark",context);
    IRBuilder<> builder(context);
    module->dump();
}