#include <vector>
using namespace std;
namespace AST{
    enum Types{
        type_int = -1,
        type_double = -2
    };

    int returnTypeOnToken(int type){
        if(type == -4) return  type_int;
        if(type == -5) return type_double;
        return 0;
    }


    const char* getTypeName(int t){
        if(t == -4) return "int";
        if(t == -5) return "double";
        return "void";
    }

    class Expression{
        Types ExpressionType;
        public:
        Expression(Types ExpressionType) : ExpressionType(ExpressionType) {}
        virtual ~Expression(){}
        virtual Types getType() = 0;
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
    };

    class DoubleNum : public Value {
        double Number;
        public:
        DoubleNum(double Number) : Number(Number) , Value(type_double){}
        Types getType() override { return type_double; }
    };

    class Variable : public Expression {
        string Name;
        public:
        Variable(const string& Name,Types VariableType) : Name(Name) , Expression(VariableType) {

        }
        const string getName(){ return Name; }
        Types getType() override {return ExpressionType;}
    };

    class Statement{
        public:
        Statement() {}
        ~Statement() {}
    };

    class CompundStatement : public Statement{
        vector<Statement> Statements;
        public:
        CompundStatement(vector<Statement> Statements) : Statements(move(Statements)) {}
    };

    class VariableDeclaration : public Statement {
        unique_ptr<Variable> var;
        unique_ptr<Expression> exp;

        public :
        VariableDeclaration(unique_ptr<Variable> var,unique_ptr<Expression> exp) : var(move(var)) , exp(move(exp)) {

        }
    };
}
