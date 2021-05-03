extern crate  llvm_sys as llvm;

use structopt::StructOpt; 
mod lexer;
mod error;
mod llvm_code_gen;
mod code_gen;
mod parser;
mod r#type;
use lexer::Lexer;
use std::fs::{File,OpenOptions};
use anyhow::{Result};
use llvm::prelude::*;
use llvm::core::*;
use llvm::LLVMLinkage;
use llvm::target::*;
use std::ffi::CString;
use std::process::Command;
use std::{mem,ptr,slice};
use tempfile::NamedTempFile;
use llvm::target_machine::LLVMCodeGenFileType::*;

#[derive(StructOpt)]
struct Cli{
    #[structopt(parse(from_os_str))]
    path : std::path::PathBuf,
}

// struct Compiler {
//     parser :   
// }

fn main() -> Result<()> {
    let args = Cli::from_args();
    let filename = args.path.to_str().unwrap().to_string();
    let  lexer = Lexer::<File>::new(args.path)?;
    // for _i in  1..39 {
    //     lexer.get_next_token()?;
    //     lexer.print_token();
    // }
    unsafe {
        LLVM_InitializeNativeTarget();
        LLVM_InitializeNativeAsmPrinter();
        LLVM_InitializeNativeAsmParser();
    };
    let target = unsafe { CString::from_raw(LLVMGetDefaultTargetTriple()) };
    let context = unsafe { LLVMContextCreate() };
    let module = unsafe { LLVMModuleCreateWithNameInContext("mod1\0".as_ptr() as *const _, context) };
    unsafe {
        let type_void = LLVMVoidTypeInContext(context);
        let func_type = LLVMFunctionType(type_void,ptr::null_mut(),0,0);
        let main_func = LLVMAddFunction(module,"main\0".as_ptr() as *const _,func_type);
        let entry_bb = LLVMAppendBasicBlock(main_func,"entry\0".as_ptr() as *const _);
        let b = LLVMCreateBuilderInContext(context);
        LLVMPositionBuilderAtEnd(b,entry_bb);
        LLVMBuildRetVoid(b);
        LLVMDisposeBuilder(b);
        let int64_ty = LLVMInt64TypeInContext(context);
        let g  = LLVMAddGlobal(module, int64_ty, "firstvar\0".as_ptr() as *const _);
        LLVMSetInitializer(g, LLVMConstInt(int64_ty, 60, 0));
        LLVMSetLinkage(g, LLVMLinkage::LLVMInternalLinkage);
        LLVMDumpModule(module);
    };
    let clon = filename.clone();
    let  outpath : Vec<&str> = clon.split(".").collect();
    let mut str : String = outpath.get(0).unwrap().to_string();
    if cfg!(target_os = "windows") { str.push_str(".exe"); }
    else { str.push_str(".out"); }
    let mut out_file = OpenOptions::new();
    out_file.create(true).write(true);
    if cfg!(unix) {
        use std::os::unix::fs::OpenOptionsExt;
        out_file.mode(0o777);
    }
    let mut outfile = out_file.open(str)?;
    let mut obj = NamedTempFile::new()?;
    let mut temp_bin = NamedTempFile::new()?;
    use llvm::target_machine::*;
    use llvm::target_machine::LLVMCodeGenOptLevel::*;
    use llvm::target_machine::LLVMRelocMode::*;
    use llvm::target_machine::LLVMCodeModel::*;
    let triple = target.as_ptr();
    unsafe {
        let mut target = mem::MaybeUninit::uninit().assume_init();
        LLVMGetTargetFromTriple(triple,&mut target,ptr::null_mut());
        let tm = LLVMCreateTargetMachine(
            target,
            triple,
            b"\0".as_ptr() as *const _,
            b"\0".as_ptr() as *const _,
            LLVMCodeGenLevelAggressive,
            LLVMRelocDefault,
            LLVMCodeModelDefault);
        let mut mbuf : LLVMMemoryBufferRef = mem::MaybeUninit::uninit().assume_init();
        LLVMTargetMachineEmitToMemoryBuffer(tm,module,LLVMObjectFile,ptr::null_mut(),&mut mbuf);
        let p = LLVMGetBufferStart( mbuf);
        let len = LLVMGetBufferSize(mbuf);
        <NamedTempFile as std::io::Write>::write_all(&mut obj,slice::from_raw_parts(p as *const _, len as usize))?;
        LLVMDisposeMemoryBuffer(mbuf);
        LLVMDisposeTargetMachine(tm);
    };
    let otc = Command::new("ld")
    .arg("-e")
    .arg("_main")
    .arg("-static")
    .arg("-o")
    .arg(temp_bin.path())
    .arg(obj.path())
    .status().expect("not able to invoke ld");

    if !otc.success() {
        panic!("linking failed : {:?}",otc);
    }
    std::io::copy(&mut temp_bin,&mut outfile).expect("Not able to copy to binary");
    unsafe { LLVMContextDispose(context) };
    return Ok(());
}
