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
        type_void = -3,
        type_err = -4,
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
        virtual void VarDecCodeGen(GlobalVariable*,Types) = 0;
    };

    class Value : public Expression {
        public:
        Value(Types ExpressionType) : Expression(ExpressionType) {} 
        void VarDecCodeGen(GlobalVariable*,Types) override;
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
        void VarDecCodeGen(GlobalVariable*,Types) override;
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
        void VarDecCodeGen(GlobalVariable*,Types) override;
        Types getType() override { return ExpressionType; }
    };



    class Statement{
        public:
        Statement() {}
        virtual ~Statement() {}
        virtual void codeGen() = 0;
        virtual bool isReturnStatement() {return false;}
    };

    class Return : public Statement{
        unique_ptr<Expression> exp;

        public :
        Return(unique_ptr<Expression> Exp) : exp(move(Exp)) {}
        bool isReturnStatement() override { return true; }
        Types getExpType(){
            if(exp) return exp->getType(); 
            return type_void;
        }
        void codeGen() override;
    };

    class CompoundStatement : public Statement{
        vector<unique_ptr<Statement>> Statements;
        public:
        CompoundStatement(vector<unique_ptr<Statement>> Statements) : Statements(move(Statements)) {}
        bool isLastElementReturnStatement() {
            if(Statements.back().get()->isReturnStatement()) return true;
            return false;
        }
        Types returnReturnStatementType(){
             if(Statements.back().get()->isReturnStatement()){
                return static_cast<Return*>(Statements.back().get())->getExpType();
             } 
             return type_err;
        }
        void codeGen() override;
    };

    class VariableDeclaration : public Statement {
        protected:
        unique_ptr<Variable> var;
        unique_ptr<Expression> exp;

        public :
        VariableDeclaration(unique_ptr<Variable> var,unique_ptr<Expression> exp) : var(move(var)) , exp(move(exp)) {

        }
       virtual void codeGen() override {};
    };


    class GlobalVariableDeclaration : public VariableDeclaration {
        public:
        GlobalVariableDeclaration(unique_ptr<Variable> var,unique_ptr<Expression> exp) : VariableDeclaration(move(var),move(exp)){}
        void codeGen() override;
    };


    class LocalVariableDeclaration : public VariableDeclaration {
        public:
        LocalVariableDeclaration(unique_ptr<Variable> var,unique_ptr<Expression> exp) : VariableDeclaration(move(var),move(exp)){}
         void codeGen() override;
    };

    class VariableAssignment : public Statement {
        unique_ptr<Variable> var;
        unique_ptr<Expression> exp;

        public :
        VariableAssignment(unique_ptr<Variable> var,unique_ptr<Expression> exp) : var(move(var)) , exp(move(exp)) {
        }
        void codeGen() override;
    };

    


    class FunctionSignature{
        string Name;
        Types retType;
        public:
        FunctionSignature(const string& Name,Types retType) : Name(Name) , retType(retType) {

        } 
        const string getName() {return Name;}
        Types getRetType(){return retType;}
    };

    class FunctionDefinition{
        unique_ptr<FunctionSignature> functionSignature;
        unique_ptr<CompoundStatement> compoundStatements;
        public:
        FunctionDefinition(unique_ptr<FunctionSignature> funcSig,unique_ptr<CompoundStatement> cmpStat) : functionSignature(move(funcSig)), compoundStatements(move(cmpStat)){
            
        }
        void codeGen();
    };
}
