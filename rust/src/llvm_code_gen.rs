use super::code_gen::Visitor;
use super::ast::*;
use llvm::prelude::*;
use llvm::core::*;
use llvm::LLVMLinkage;



struct LLVMCodeGenVisitor {
    context : LLVMContextRef,
    int64_ty : LLVMTypeRef,
    llvm_val : LLVMValueRef,
    module : LLVMModuleRef,
}

impl Visitor for LLVMCodeGenVisitor {
    type Exp = LLVMValueRef;

    fn int_value_node(&mut self,node : &mut Int) { 
        self.llvm_val = unsafe { LLVMConstInt(self.int64_ty,node.val,0) };
    }

    fn global_variable_declaration_codegen<T : Value>(&mut self,v: &mut VariableDeclaration<T>)  {
        unsafe {
            let gvar = LLVMAddGlobal(self.module,self.int64_ty,v.name.as_ptr() as *const _);
            v.value.accept(self);
            LLVMSetInitializer(gvar,self.llvm_val);
            LLVMSetLinkage(gvar,LLVMLinkage::LLVMInternalLinkage);
        };
    }

}