use llvm::prelude::*;


trait Visitor{
    type Exp;
    fn variable_declaration_codegen() -> Option<Self::Exp>;
}