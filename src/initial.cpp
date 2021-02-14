#include <stdio.h>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
using namespace std;
using namespace llvm;
#include "parser.h"
extern unique_ptr<Module> module;

int main(){
    
   

    Parser parser("srcf.qk");
    parser.parse();

    if(verifyModule(*module)){
        printf("Error in CodeGen");
    }

    auto TargetTriple = sys::getDefaultTargetTriple();

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();


    string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple,Error);

    if(!Target){
        errs() << Error;
        return 1;
    }
    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple,CPU,Features,opt,RM);
    module->setDataLayout(TargetMachine->createDataLayout());
    module->setTargetTriple(TargetTriple);

    auto Filename = "output.o";
    error_code EC;
    raw_fd_ostream dest(Filename,EC,sys::fs::OF_None);
    if(EC)
    {
        errs() << "Could not open file: " << EC.message();
        return 0;
    }
    legacy::PassManager pass;
    auto FileType = CGFT_ObjectFile;
    if(TargetMachine->addPassesToEmitFile(pass,dest,nullptr,FileType)){
        errs() << "TargetMachine can't emit a file of this type";
        return 0;
    }
    pass.run(*module);

    dest.flush();
    dest.close();
    printf("Successfully Compiled");
    return 0;
}   