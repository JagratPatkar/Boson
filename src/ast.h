#include<string>
#include <vector>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/Module.h"
using namespace llvm;
using namespace std;
namespace AST{

    enum Types{
        type_int = -1,
        type_double = -2,
        type_void = -3
    };


    enum BinOps{
        op_add = -1,
        op_sub = -2,
        op_mul = -3,
        op_div = -4,
        non_op = -5,
    };

    Types TypesOnToken(int type);

    const char* TypesName(int t);
   

    class Expression{
        protected:
        Types ExpressionType;
        public:
        Expression(Types ExpressionType) : ExpressionType(ExpressionType) {}
        virtual ~Expression(){}
        virtual Types getType() = 0;
        virtual llvm::Value* codeGen() =0;
    };

    class Value : public Expression {
        public:
        Value(Types ExpressionType) : Expression(ExpressionType) {} 
    };

    class IntNum : public Value {
        int Number;
        public :
        IntNum(int Number) : Number(Number) , Value(type_int){} 
        Types getType() override { return type_int; }
        llvm::Value* codeGen() override;
    };

    class DoubleNum : public Value {
        double Number;
        public:
        DoubleNum(double Number) : Number(Number) , Value(type_double){}
        Types getType() override { return type_double; }
        llvm::Value* codeGen() override;
    };

    class Variable : public Expression {
        string Name;
        public:
        Variable(const string& Name,Types VariableType) : Name(Name) , Expression(VariableType) {

        }
        const string getName(){ return Name; }
        Types getType() override { return ExpressionType; }
        llvm::Value* codeGen() override;
    };


    class BinaryExpression : public Expression{
        BinOps op;
        unique_ptr<Expression> LVAL;
        unique_ptr<Expression> RVAL;
        public:
        BinaryExpression(BinOps op,unique_ptr<Expression> LVAL,unique_ptr<Expression> RVAL,Types ExpressionType) : op(op) , LVAL(move(LVAL)) , RVAL(move(RVAL)) , Expression(ExpressionType) {

        }
        llvm::Value* codeGen() override;
        Types getType() override { return ExpressionType; }
    };



    class Statement{
        public:
        Statement() {}
        virtual ~Statement() {}
        virtual void codeGen() = 0;
    };

    class CompundStatement : public Statement{
        vector<Statement> Statements;
        public:
        CompundStatement(vector<Statement> Statements) : Statements(move(Statements)) {}
        void codeGen();
    };

    class VariableDeclaration : public Statement {
        unique_ptr<Variable> var;
        unique_ptr<Expression> exp;

        public :
        VariableDeclaration(unique_ptr<Variable> var,unique_ptr<Expression> exp) : var(move(var)) , exp(move(exp)) {

        }
        void codeGen();
    };

    class VariableAssignment : public Statement {
        unique_ptr<Variable> var;
        unique_ptr<Expression> exp;

        public :
        VariableAssignment(unique_ptr<Variable> var,unique_ptr<Expression> exp) : var(move(var)) , exp(move(exp)) {
        }
        void codeGen();
    };
}
