#include <string>
#include <vector>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/Module.h"
#include "../codegen/codegen.h"
using namespace llvm;
using namespace std;
namespace AST
{

    enum Types
    {
        type_int = -1,
        type_double = -2,
        type_void = -3,
        type_err = -4,
    };

    class BinOps
    {
    protected:
        CodeGen *cg;

    public:
        BinOps()
        {
            cg = CodeGen::GetInstance();
        }
        virtual llvm::Value *codeGenInt(llvm::Value*, llvm::Value*) = 0;
        virtual llvm::Value *codeGenDouble(llvm::Value*, llvm::Value*) = 0;
        virtual ~BinOps() {}
    };

    class OpAdd : public BinOps
    {
        llvm::Value* codeGenInt(llvm::Value* lhs, llvm::Value* rhs) override
        {
            return cg->builder->CreateAdd(lhs, rhs, "additmp");
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs, llvm::Value* rhs) override
        {
            return cg->builder->CreateFAdd(lhs, rhs, "addftmp");
        }
    };

    class OpSub : public BinOps
    {
        llvm::Value* codeGenInt(llvm::Value* lhs, llvm::Value* rhs) override
        {
            return cg->builder->CreateSub(lhs, rhs, "subitmp");
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs, llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFSub(lhs, rhs, "subftmp"); 
        }
    };

    class OpMul : public BinOps
    {
        llvm::Value* codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateMul(lhs, rhs, "mulitmp"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFMul(lhs, rhs, "mulftmp"); 
        }
    };

    class OpDiv : public BinOps
    {
        llvm::Value* codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateSDiv(lhs, rhs, "divitmp"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFDiv(lhs, rhs, "divftmp"); 
        }
    };

    class OpLessThan : public BinOps
    {
        llvm::Value *codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateICmpULT(lhs, rhs, "ltcmpi"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFCmpULT(lhs, rhs, "ltcmpd"); 
        }
    };

    class OpGreaterThan : public BinOps
    {
        llvm::Value *codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateICmpUGT(lhs, rhs, "gtcmpi"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFCmpUGT(lhs, rhs, "gtcmpd"); 
        }
    };

    class OpLessThanEq : public BinOps
    {
        llvm::Value *codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateICmpULE(lhs, rhs, "lecmpi"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFCmpULE(lhs, rhs, "lecmpd"); 
        }
    };

    class OpGreaterThanEq : public BinOps
    {
        llvm::Value *codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateICmpUGE(lhs, rhs, "gecmpi"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFCmpUGE(lhs, rhs, "gecmpd");
        }
    };

    class OpEqualTo : public BinOps
    {
        llvm::Value *codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateICmpEQ(lhs, rhs, "eqi"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFCmpUEQ(lhs, rhs, "eqcmpd"); 
        }
    };

    class OpNotEqualTo : public BinOps
    {
        llvm::Value *codeGenInt(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateICmpNE(lhs, rhs, "neqcmpi"); 
        }
        llvm::Value *codeGenDouble(llvm::Value* lhs,llvm::Value* rhs) override 
        { 
            return cg->builder->CreateFCmpUNE(lhs, rhs, "neqcmpd"); 
        }
    };

    Types TypesOnToken(int type);

    const char *TypesName(int t);

    class Expression
    {
    protected:
        Types ExpressionType;

    public:
        Expression(Types ExpressionType) : ExpressionType(ExpressionType) {}
        virtual ~Expression() {}
        virtual Types getType() = 0;
        virtual llvm::Value *codeGen() = 0;
        virtual void VarDecCodeGen(GlobalVariable *, Types) = 0;
    };

    class Value : public Expression
    {
    public:
        Value(Types ExpressionType) : Expression(ExpressionType) {}
        void VarDecCodeGen(GlobalVariable *, Types) override;
    };

    class IntNum : public Value
    {
        int Number;

    public:
        IntNum(int Number) : Number(Number), Value(type_int) {}
        Types getType() override { return type_int; }
        llvm::Value *codeGen() override;
    };

    class DoubleNum : public Value
    {
        double Number;

    public:
        DoubleNum(double Number) : Number(Number), Value(type_double) {}
        Types getType() override { return type_double; }
        llvm::Value *codeGen() override;
    };

    class Variable : public Expression
    {
        string Name;

    public:
        Variable(const string &Name, Types VariableType) : Name(Name), Expression(VariableType)
        {
        }
        const string getName() { return Name; }
        Types getType() override { return ExpressionType; }
        void VarDecCodeGen(GlobalVariable *, Types) override;
        llvm::Value *codeGen() override;
    };

    class BinaryExpression : public Expression
    {
        unique_ptr<BinOps> op;
        unique_ptr<Expression> LVAL;
        unique_ptr<Expression> RVAL;

    public:
        BinaryExpression(unique_ptr<BinOps> op, unique_ptr<Expression> LVAL, unique_ptr<Expression> RVAL, Types ExpressionType) : op(move(op)), LVAL(move(LVAL)), RVAL(move(RVAL)), Expression(ExpressionType)
        {
        }
        llvm::Value *codeGen() override;
        void VarDecCodeGen(GlobalVariable *, Types) override;
        Types getType() override { return ExpressionType; }
    };

    class Statement
    {
    public:
        Statement() {}
        virtual ~Statement() {}
        virtual void codegen() = 0;
        virtual bool isReturnStatement() { return false; }
    };

    class Return : public Statement
    {
        unique_ptr<Expression> exp;

    public:
        Return(unique_ptr<Expression> Exp) : exp(move(Exp)) {}
        bool isReturnStatement() override { return true; }
        Types getExpType()
        {
            if (exp)
                return exp->getType();
            return type_void;
        }
        void codegen() override;
    };

    class CompoundStatement : public Statement
    {
        vector<unique_ptr<Statement>> Statements;

    public:
        CompoundStatement(vector<unique_ptr<Statement>> Statements) : Statements(move(Statements)) {}
        bool isLastElementReturnStatement()
        {
            if (Statements.back().get()->isReturnStatement())
                return true;
            return false;
        }
        Types returnReturnStatementType()
        {
            if (Statements.back().get()->isReturnStatement())
            {
                return static_cast<Return *>(Statements.back().get())->getExpType();
            }
            return type_err;
        }
        void codegen() override;
    };

    class VariableDeclaration : public Statement
    {
    protected:
        unique_ptr<Variable> var;
        unique_ptr<Expression> exp;

    public:
        VariableDeclaration(unique_ptr<Variable> var, unique_ptr<Expression> exp) : var(move(var)), exp(move(exp))
        {
        }
        virtual void codegen() override{};
    };

    class GlobalVariableDeclaration : public VariableDeclaration
    {
    public:
        GlobalVariableDeclaration(unique_ptr<Variable> var, unique_ptr<Expression> exp) : VariableDeclaration(move(var), move(exp)) {}
        void codegen() override;
    };

    class LocalVariableDeclaration : public VariableDeclaration
    {
    public:
        LocalVariableDeclaration(unique_ptr<Variable> var, unique_ptr<Expression> exp) : VariableDeclaration(move(var), move(exp)) {}
        void codegen() override;
    };

    class VariableAssignment : public Statement
    {
        unique_ptr<Variable> var;
        unique_ptr<Expression> exp;

    public:
        VariableAssignment(unique_ptr<Variable> var, unique_ptr<Expression> exp) : var(move(var)), exp(move(exp))
        {
        }
        void codegen() override;
    };

    class FunctionDefinition;

    class FunctionSignature
    {
        string Name;
        Types retType;
        vector<unique_ptr<Variable>> args;
        friend FunctionDefinition;

    public:
        FunctionSignature(const string &Name, Types retType, vector<unique_ptr<Variable>> args) : Name(Name), retType(retType), args(move(args))
        {
        }
        const string getName() { return Name; }
        Types getRetType() { return retType; }
        void codegen();
    };

    class FunctionDefinition
    {
        unique_ptr<FunctionSignature> functionSignature;
        unique_ptr<CompoundStatement> compoundStatements;

    public:
        FunctionDefinition(unique_ptr<FunctionSignature> funcSig, unique_ptr<CompoundStatement> cmpStat) : functionSignature(move(funcSig)), compoundStatements(move(cmpStat))
        {
        }
        void codeGen();
    };

    class FunctionCall : public Expression, public Statement
    {
        string Name;
        vector<unique_ptr<Expression>> args;

    public:
        FunctionCall(const string &Name, vector<unique_ptr<Expression>> args, Types type) : Expression(type), Name(Name), args(move(args))
        {
        }
        void codegen() override;
        llvm::Value *codeGen() override;
        Types getType() override { return ExpressionType; }
        void VarDecCodeGen(GlobalVariable *, Types) override;
    };

    class IfElseStatement : public Statement
    {
        unique_ptr<Expression> Condition;
        unique_ptr<CompoundStatement> compoundStatements;
        unique_ptr<CompoundStatement> elseCompoundStatements;

    public:
        IfElseStatement(unique_ptr<Expression> cond, unique_ptr<CompoundStatement> cmpStat, unique_ptr<CompoundStatement> ecmpStat) : Condition(move(cond)), compoundStatements(move(cmpStat)), elseCompoundStatements(move(ecmpStat))
        {
        }
        void codegen() override;
    };

    class ForStatement : public Statement
    {
        unique_ptr<LocalVariableDeclaration> lvd;
        unique_ptr<VariableAssignment> va;
        unique_ptr<Expression> cond;
        unique_ptr<VariableAssignment> vastep;
        unique_ptr<CompoundStatement> compoundStatement;

    public:
        ForStatement(unique_ptr<LocalVariableDeclaration> lvd, unique_ptr<VariableAssignment> va, unique_ptr<Expression> cond, unique_ptr<VariableAssignment> vastep, unique_ptr<CompoundStatement> compoundStatement) : lvd(move(lvd)), va(move(va)), cond(move(cond)), vastep(move(vastep)), compoundStatement(move(compoundStatement))
        {
        }
        void codegen() override;
    };
} // namespace AST