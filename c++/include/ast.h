#include <string>
#include <memory>
#include <vector>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/Module.h"
#include "codegen.h"
#include "type.h"
using namespace llvm;
using namespace std;
namespace AST
{

    class BinOps
    {
    protected:
        CodeGen *cg;
        unique_ptr<::Type> operand_type;

    public:
        BinOps()
        {
            cg = CodeGen::GetInstance();
        }

        virtual bool validOperandSet(::Type *) = 0;
        virtual llvm::Value *codeGen(llvm::Value *, llvm::Value *) = 0;
        virtual unique_ptr<::Type> getOperatorEvalTy() = 0;
        virtual ~BinOps() {}
    };

    class OpAdd : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateAdd(lhs, rhs, "additmp");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFAdd(lhs, rhs, "addftmp");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();
            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override
        {
            return operand_type->getNew();
        }
    };

    class OpSub : public BinOps
    {
        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateSub(lhs, rhs, "subitmp");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFSub(lhs, rhs, "subftmp");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();
            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override
        {
            return operand_type->getNew();
        }
    };

    class OpMul : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateMul(lhs, rhs, "mulitmp");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFMul(lhs, rhs, "mulftmp");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();
            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override
        {
            return operand_type->getNew();
        }
    };

    class OpDiv : public BinOps
    {
        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateSDiv(lhs, rhs, "divitmp");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFDiv(lhs, rhs, "divftmp");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();
            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override
        {
            return operand_type->getNew();
        }
    };

    class OpLessThan : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateICmpSLT(lhs, rhs, "ltcmpi");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFCmpULT(lhs, rhs, "ltcmpd");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();

            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };

    class OpGreaterThan : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateICmpSGT(lhs, rhs, "gtcmpi");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFCmpUGT(lhs, rhs, "gtcmpd");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();

            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };

    class OpLessThanEq : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateICmpSLE(lhs, rhs, "lecmpi");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFCmpULE(lhs, rhs, "lecmpd");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();

            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };

    class OpGreaterThanEq : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateICmpSGE(lhs, rhs, "gecmpi");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFCmpUGE(lhs, rhs, "gecmpd");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble());
            if (c)
                operand_type = t1->getNew();

            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };

    class OpEqualTo : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt() || operand_type->isBool())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateICmpEQ(lhs, rhs, "eqi");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFCmpUEQ(lhs, rhs, "eqcmpd");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble() || t1->isBool());
            if (c)
                operand_type = t1->getNew();

            return c;
        }
        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };

    class OpNotEqualTo : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            if (operand_type->isInt() || operand_type->isBool())
                return codeGenInt(lhs, rhs);
            else if (operand_type->isDouble())
                return codeGenDouble(lhs, rhs);
            return nullptr;
        }

        llvm::Value *codeGenInt(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateICmpNE(lhs, rhs, "neqcmpi");
        }
        llvm::Value *codeGenDouble(llvm::Value *lhs, llvm::Value *rhs)
        {
            return cg->builder->CreateFCmpUNE(lhs, rhs, "neqcmpd");
        }

        bool validOperandSet(::Type *t1) override
        {
            bool c = (t1->isInt() || t1->isDouble() || t1->isBool());
            if (c)
                operand_type = t1->getNew();
            return c;
        }

        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };

    class OpAnd : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            return cg->builder->CreateAnd(lhs, rhs, "andop");
        }

        bool validOperandSet(::Type *t1) override
        {
            return t1->isBool();
        }

        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };

    class OpOr : public BinOps
    {

        llvm::Value *codeGen(llvm::Value *lhs, llvm::Value *rhs) override
        {
            return cg->builder->CreateOr(lhs, rhs, "orop");
        }

        bool validOperandSet(::Type *t1) override
        {
            return t1->isBool();
        }

        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
    };


    class Expression
    {
    protected:
        unique_ptr<::Type> type;

    public:
        Expression(unique_ptr<::Type> type) : type(move(type))
        {
        }
        virtual ~Expression() {}
        virtual ::Type *getType()
        {
            return type.get();
        }
        virtual llvm::Value *codeGen() = 0;
        virtual bool isValue() { return false; }
        virtual bool isVariable() { return false; }
        virtual void VarDecCodeGen(GlobalVariable *, ::Type *) = 0;
    };

    class Value : public Expression
    {
    public:
        Value(unique_ptr<::Type> type) : Expression(move(type)) {}
        virtual void VarDecCodeGen(GlobalVariable *, ::Type *) override;
        virtual bool isValue() override { return true; }
    };

    class IntNum : public Value
    {
        int Number;

    public:
        IntNum(int Number) : Number(Number), Value(make_unique<Int>()) {}
        llvm::Value *codeGen() override { return ConstantInt::get(type->getLLVMType(), Number, true); }
    };

    class DoubleNum : public Value
    {
        double Number;

    public:
        DoubleNum(double Number) : Number(Number), Value(make_unique<Double>()) {}
        llvm::Value *codeGen() override { return ConstantFP::get(type->getLLVMType(), Number); }
    };

    class Boolean : public Value
    {
        bool value;

    public:
        Boolean(bool value) : value(value), Value(make_unique<Bool>()) {}
        llvm::Value *codeGen() override { return ConstantInt::get(type->getLLVMType(), value, true); }
    };

    class ArrayVal : public Value
    {
        vector<unique_ptr<Expression>> ofVals;

    public:
        ArrayVal(vector<unique_ptr<Expression>> ofVals, unique_ptr<::Type> type, int size) : ofVals(move(ofVals)), Value(make_unique<Array>(size, move(type)))
        {
        }
        llvm::Value *codeGen() override;
        void gen(llvm::Value *);
        void VarDecCodeGen(GlobalVariable *, ::Type *) override;
    };

    class Variable : public Expression
    {
        string Name;
        int elemNum;
        bool isArrayElem;
        unique_ptr<::Type> arrayType;
        unique_ptr<Expression> elem;

    public:
        Variable(const string &Name, unique_ptr<::Type> type) : Name(Name), Expression(move(type))
        {
            isArrayElem = false;
        }
        const string getName() { return Name; }
        void VarDecCodeGen(GlobalVariable *, ::Type *) override;
        void setElement(unique_ptr<Expression> e) { elem = move(e); }
        llvm::Value *getElement()
        {
            if (elem)
                return elem->codeGen();
            return nullptr;
        }
        void setArrayType(unique_ptr<::Type> t) { arrayType = move(t); }
        void setArrayFlag() { isArrayElem = true; }
        bool isVariable() override { return true; }
        llvm::Value *codeGen() override;
    };

    class BinaryExpression : public Expression
    {
        unique_ptr<BinOps> op;
        unique_ptr<Expression> LVAL;
        unique_ptr<Expression> RVAL;

    public:
        BinaryExpression(unique_ptr<BinOps> op, unique_ptr<Expression> LVAL, unique_ptr<Expression> RVAL, unique_ptr<::Type> ExpressionType) : op(move(op)), LVAL(move(LVAL)), RVAL(move(RVAL)), Expression(move(ExpressionType))
        {
        }
        llvm::Value *codeGen() override;
        void VarDecCodeGen(GlobalVariable *, ::Type *) override;
    };

      class UnOps {
        protected:
        CodeGen* cg;
        unique_ptr<::Type> op_type;
        public:
        UnOps(){
            cg = CodeGen::GetInstance();
        }
        virtual bool validOperandSet(Expression * e) = 0;
        virtual llvm::Value *codeGen(llvm::Value*,Expression* e) = 0;
        virtual unique_ptr<::Type> getOperatorEvalTy() = 0;
        virtual ~UnOps() {}
    };


    class AddPostIncrement : public UnOps {
        public:
        bool validOperandSet(Expression * e) override {
            bool c = (e->getType()->isInt() || e->getType()->isDouble()) && e->isVariable();
            if(c) op_type = e->getType()->getNew();
            return c;
        }
        
        unique_ptr<::Type> getOperatorEvalTy() override { return op_type.get()->getNew(); }
        llvm::Value* codeGen(llvm::Value*,Expression* e) override;
    };


    class SubPostIncrement : public UnOps {
        public:
        bool validOperandSet(Expression * e) override {
            bool c = (e->getType()->isInt() || e->getType()->isDouble()) && e->isVariable();
            if(c) op_type = e->getType()->getNew();
            return c;
        }
        
        unique_ptr<::Type> getOperatorEvalTy() override { return op_type.get()->getNew(); }
        llvm::Value* codeGen(llvm::Value*,Expression* e) override;
    };

    class AddPreIncrement : public UnOps {
        public:
        bool validOperandSet(Expression * e) override {
            bool c = (e->getType()->isInt() || e->getType()->isDouble()) && e->isVariable();
            if(c) op_type = e->getType()->getNew();
            return c;
        }
        
        unique_ptr<::Type> getOperatorEvalTy() override { return op_type.get()->getNew(); }
        llvm::Value* codeGen(llvm::Value*,Expression* e) override;
    };


    class SubPreIncrement : public UnOps {
        public:
        bool validOperandSet(Expression * e) override {
            bool c = (e->getType()->isInt() || e->getType()->isDouble()) && e->isVariable();
            if(c) op_type = e->getType()->getNew();
            return c;
        }
        
        unique_ptr<::Type> getOperatorEvalTy() override { return op_type.get()->getNew(); }
        llvm::Value* codeGen(llvm::Value*,Expression* e) override;
    };


    class PreNot : public UnOps {
        public:
        bool validOperandSet(Expression * e) override {
            bool c = e->getType()->isBool();
            if(c) op_type = e->getType()->getNew();
            return c;
        }
        
        unique_ptr<::Type> getOperatorEvalTy() override { return make_unique<Bool>(); }
        llvm::Value* codeGen(llvm::Value* dest,Expression* e) override { 
            if(e->isVariable()){
                return op_type->createNot(e->codeGen());
            }
            return op_type->createNot(dest); 
        }
    };

    class Neg : public UnOps {
        public:
        bool validOperandSet(Expression * e) override {
            bool c = e->getType()->isInt() || e->getType()->isDouble();
            if(c) op_type = e->getType()->getNew();
            return c;
        }
        unique_ptr<::Type> getOperatorEvalTy() override { return op_type.get()->getNew(); }
        llvm::Value* codeGen(llvm::Value*,Expression* e) override;
    };



    class UnaryExpression : public Expression {
        unique_ptr<UnOps> op;
        unique_ptr<Expression> VAL;

        public:
        UnaryExpression(unique_ptr<UnOps> op,unique_ptr<Expression> VAL,unique_ptr<::Type> ExpressionType) : op(move(op)) , VAL(move(VAL)) , Expression(move(ExpressionType)) {}
        llvm::Value *codeGen() override;
        void VarDecCodeGen(GlobalVariable *, ::Type *) override;
    };

    class Statement
    {
        bool oneAhead;
    public:
        Statement()
        {
            oneAhead = false;
        }
        virtual ~Statement() {}
        virtual void codegen() = 0;
        virtual bool isReturnStatement() { return false; }
        virtual void peekedOneAhead(bool t){ oneAhead = t; }
        virtual bool didPeekOneAhead() { return oneAhead; }
    };

    class Return : public Statement
    {
        unique_ptr<Expression> exp;

    public:
        Return(unique_ptr<Expression> Exp) : exp(move(Exp)) {}
        bool isReturnStatement() override { return true; }
        ::Type *getExpType()
        {
            if (exp)
                return exp->getType();
            return new Void();
        }
        void codegen() override;
    };

    class CompoundStatement : public Statement
    {
        vector<unique_ptr<Statement>> Statements;
        bool hasRet;
    public:
        CompoundStatement(vector<unique_ptr<Statement>> Statements) : Statements(move(Statements)) {
            hasRet = false;
        }
        bool isLastElementReturnStatement()
        {
            if (Statements.back().get()->isReturnStatement())
                return true;
            return false;
        }
        bool getHasRet(){ return hasRet; }
        void setHasRetTrue(){ hasRet = true; }
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
        unique_ptr<::Type> retType;
        vector<unique_ptr<Variable>> args;
        friend FunctionDefinition;

    public:
        FunctionSignature(const string &Name, unique_ptr<::Type> retType, vector<unique_ptr<Variable>> args) : Name(Name), retType(move(retType)), args(move(args))
        {
        }
        const string getName() { return Name; }
        ::Type *getRetType() { return retType.get(); }
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
        FunctionCall(const string &Name, vector<unique_ptr<Expression>> args, unique_ptr<::Type> type) : Expression(move(type)), Name(Name), args(move(args))
        {
        }
        void codegen() override;

        llvm::Value *codeGen() override;
        void VarDecCodeGen(GlobalVariable *, ::Type *) override;
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