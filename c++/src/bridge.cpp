#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string.h>
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

class Bridge
{
    string FileName;
    Parser parser;
    CodeGen *cg;

public:
    Bridge(const string &fn) : FileName(fn), parser(fn)
    {
        cg = CodeGen::GetInstance();
    }
    void startCompilation() { parser.parse(); }
    void verifyIR()
    {
        if (verifyModule(*(cg->module)))
            cerr << "Error in CodeGen " << endl;
    }

    void createObjFile();

    ~Bridge()
    {
        delete cg;
    }
};

void Bridge::createObjFile()
{
    auto TargetTriple = sys::getDefaultTargetTriple();

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target)
    {
        cerr << Error;
        return;
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    cg->module->setDataLayout(TargetMachine->createDataLayout());
    cg->module->setTargetTriple(TargetTriple);
    char delim = '.';
    char *name = strtok(&(FileName[0]), &delim);
    
    string filename(name);
    filename += ".o";
    error_code EC;
    raw_fd_ostream dest(filename, EC);
    if(EC){
        cerr << "Could not open file: " << EC.message() << endl;
        return;
    }

    legacy::PassManager pass;
    auto FileType = CGFT_ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        cerr << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*(cg->module));
    dest.flush();
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Target File Not Specified" << endl;
        return 0;
    }
    string FileName(argv[1]);
    Bridge bridge(FileName);
    bridge.startCompilation();
    bridge.verifyIR();
    bridge.createObjFile();
    printf("Successfully Compiled");
    return 0;
}