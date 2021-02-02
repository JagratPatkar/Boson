#include "parser.h"



void Parser::parse(){
    int token = lexer.getNextToken();
    while(true) 
    {   
        switch(token){
            case -2 : 
            if(ParseIntNum()){
                 printf("Parsed an Integer Number Sucessfully \n"); 
            }
            break;
            case -3 : 
            if(ParseDoubleNum()){
                 printf("Parsed an Double Number Sucessfully \n"); 
            }
            break;
            case -7 :
                if(ParseIdentifier()){

                }
            break;
            case -1 : printf("End of File \n");
                    exit(1);
                     break;
        }
        token = lexer.getNextToken();
    }
    lexer.closeFile();
}  

unique_ptr<Expression> Parse::ParseVariableDeclaration(){
    int type = lexer.getCurrentToken();
    lexer.getNextToken();
    if(!lexer.isTokenIdentifier()) return LogError("Identifier Expected"); 
    string name = lexer.getIdentifier();
    type = returnTypeOnToken(type);
    auto var = make_unique<Variable>(name,type);
    lexer.getNextToken();
    if(!lexer.isTokenAssignmentOp()) return LogError("Missing Assignment Operator");
    lexer.getNextToken();
    auto exp = ParseExpression();
    int t1 = var.getType();
    int t2 = exp.getType();
    if(t1 != t2) return LogTypeError(t1,t2);
    return make_unique<VariableDeclaration>(var,exp);
}

unique_ptr<Expression> Parse::LogError(const char* errmsg){
    fprintf("Error : %s\n",errmsg);
    return nullptr;
}

unqiue_ptr<Expression> Parse::LogTypeError(int t1,int t2){
    fprintf("Type mistmatch between : %s - and - %s", getTypeName(t1) , getTypeName(t2));
    return nullptr;
}

unique_ptr<Expression> ParseExpression(){
    auto lvalue = ParsePrimary();
    if(!lvalue) return nullptr;
    return lvalue;
}

unique_ptr<Expression> Parse::ParsePrimary(){
    if(lexer.isTokenIntNum()) return ParseIntNum();
    if(lexer.isTokenDoubleNum()) return ParseDoubleNum();
    if(lexer.isTokenIdentifier()) return ParseIdentifier();
    else return LogError("Unknown Expression!");
}   


unique_ptr<Expression> Parse::ParseIdentifier(){
    string Name = lexer.getIdentifier();
    lexer.getNextToken();
    // return make_unique<Variable>(Name);
    return null;
}

unique_ptr<Expression>  Parser::ParseIntNum(){
    return make_unique<IntNum>(lexer.getIntNum());
}

unique_ptr<Expression> Parser::ParseDoubleNum(){
    return make_unique<DoubleNum>(lexer.getDoubleNum());
}