use llvm::prelude::*;
use super::r#type::Types;
use super::code_gen::Visitor;

pub trait Node {
     fn accept<T : Visitor>(&mut self,visitor : &mut T);
}
trait Statement : Node { }
trait Expression : Node { }
pub trait Value : Node {  }

pub struct Int{
    pub val : u64
}


impl Node for Int{
    fn accept<T : Visitor>(&mut self,visitor : &mut T){

    }
}

impl Value for Int {
}

pub struct VariableDeclaration<T> where T : Value{
    pub r#type : Types,
    pub name : String,
    pub value : T
}