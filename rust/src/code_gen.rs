use super::ast::*;

pub trait Visitor{
    type Exp;
    fn int_value_node(&mut self,node : &mut Int);
    fn global_variable_declaration_codegen<T : Value>(&mut self,v: &mut VariableDeclaration<T>);
}