using namespace std;
#include <fstream>

namespace Lexer{
    class Lexer{
        enum token{
            token_number = -2,
            token_eof = -1,
        };
        string sourceName;
        std::ifstream source;
        public:
        Lexer(string name) : sourceName(name) {
            source.open(sourceName);
        } 
        int getToken();
    };
}