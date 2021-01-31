namespace AST{
    enum Types{
        type_int = -1,
        type_double = -2
    };
    class Expression{
        Types ExpressionType;
        public:
        Expression(Types ExpressionType) : ExpressionType(ExpressionType) {}
        virtual ~Expression(){}
    };

    class Value : public Expression {
        public:
        Value(Types ExpressionType) : Expression(ExpressionType) {} 
        virtual Types getType() = 0;
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
}
