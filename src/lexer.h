#include <fstream>
using namespace std;
class Lexer
{
    enum token
    {
        token_int_num = -2,
        token_double_num = -3,
        token_eof = -1,
        token_int = -4,
        token_double = -5,
        token_assignment_op = -6,
        token_identifier = -7,
    };
    string sourceName;
    std::ifstream source;
    double DoubleNum;
    int IntNum;
    string NumberString;
    string Identifier;
    int currentToken;
public:
    Lexer(string name) : sourceName(name){
        source.open(sourceName);
    }
    int getToken();
    double getDoubleNum(){ return DoubleNum; }
    int getIntNum(){ return IntNum; }
    string getIdentifier() {return Identifier.c_str();}
    int getNextToken();
    int getCurrentToken();
    void closeFile() { source.close(); }
};
