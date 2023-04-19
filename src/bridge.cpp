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
        std::string errorMessage;
        llvm::raw_string_ostream errorStream(errorMessage);
        if (llvm::verifyModule(*(cg->module), &errorStream)) 
            llvm::errs() << "Error: " << errorMessage << "\n";
        //cg->module->print(llvm::outs(), nullptr);
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
    string filename = FileName.substr(0, FileName.find_last_of('.'));
    filename += ".o";
    error_code EC;
    raw_fd_ostream dest(filename, EC, sys::fs::OF_None);
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

    // Check if the file has the .b extension
    if (FileName.substr(FileName.find_last_of('.')).compare(".b") != 0)
    {
        cerr << "Error: File must have the .b extension" << endl;
        return 0;
    }

    // Check if the file exists
    std::ifstream file(FileName);
    if (!file.good())
    {
        cerr << "Error: File does not exist or cannot be opened" << endl;
        return 0;
    }
    file.close();

    Bridge bridge(FileName);
    bridge.startCompilation();
    bridge.verifyIR();
    bridge.createObjFile();
    printf("Successfully Compiled");
    return 0;
}