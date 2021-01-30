#include <stdio.h>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
using namespace std;
#include "lexer.h"

using namespace llvm;

unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
unique_ptr<Module> module = make_unique<Module>("quark",*context);


int main(){
    
    // IRBuilder<> builder(*context);
    // FunctionType *funcType = FunctionType::get(builder.getInt32Ty(),false);
    // Function *mainFunction = Function::Create(funcType,Function::ExternalLinkage,"main",*module);
    
    // BasicBlock *entry = BasicBlock::Create(*context,"entrypoint",mainFunction);
    // builder.SetInsertPoint(entry);

    // Value *helloworld = builder.CreateGlobalStringPtr("quark lang\n");
    // vector<Type*> putsArg;
    // putsArg.push_back(builder.getInt8Ty()->getPointerTo()); 
    // ArrayRef<Type*> argsRef(putsArg);
    // FunctionType *putsType = FunctionType::get(builder.getInt32Ty(),argsRef,false);
    // FunctionCallee putsFunc = module->getOrInsertFunction("puts",putsType);
    // builder.CreateCall(putsFunc,helloworld);
    // builder.CreateRetVoid();
    // module->dump();


    Lexer lexer("srcf.qk");
    int token = lexer.getToken();
    printf("Token = %d \n",token); 
    while(true) 
    {   
        switch(token){
            case -2 : printf("IntNum = %d \n",lexer.getIntNum()); break;
            case -3 : printf("DoubleNum = %f \n",lexer.getDoubleNum()); break;
            case -1 : printf("End of File \n");
                      return 1;
                     break;
        }
        token = lexer.getToken();
        printf("Token = %d \n",token); 
    }
    
    lexer.closeFile();
}