using namespace std;
#include <fstream>

namespace Lexer{
    class Lexer{
        enum token{
            token_number = -1,
        };
        string sourceName;
        std::ifstream source;
        public:
        Lexer(string name) : sourceName(name) {
            
        } 
    };
}