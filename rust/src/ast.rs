use llvm::prelude::*;
use super::type::types;

trait Node {
    fn accept<T : Visitor>(visitor : T);
}
trait Statement : Node { }
trait Expression : Node { }
trait Value : Node { type ValTyp;  }

struct VariableDeclaration<T> where T : Value{
    type : Types,
    name : String,
    value : T
}

