#include "parser.h"



void Parser::parse(){
    int token = lexer.getToken();
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
            case -1 : printf("End of File \n");
                    exit(1);
                     break;
        }
        token = lexer.getToken();
    }
    lexer.closeFile();
}  

unique_ptr<Expression>  Parser::ParseIntNum(){
    return make_unique<IntNum>(lexer.getIntNum());
}

unique_ptr<Expression> Parser::ParseDoubleNum(){
    return make_unique<DoubleNum>(lexer.getDoubleNum());
}

