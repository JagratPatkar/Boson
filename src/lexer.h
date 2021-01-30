#include <fstream>
using namespace std;
class Lexer
{
    enum token
    {
        token_int_num = -2,
        token_double_num = -3,
        token_eof = -1,
    };
    string sourceName;
    std::ifstream source;
    double DoubleNum;
    int IntNum;
    string NumberString;
    int currentToken;
public:
    Lexer(string name) : sourceName(name){
        source.open(sourceName);
    }
    int getToken();
    double getDoubleNum(){ return DoubleNum; }
    int getIntNum(){ return IntNum; }
    int getNextToken();
    void closeFile() { source.close(); }
};
