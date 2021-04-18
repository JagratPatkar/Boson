use thiserror::Error;
use utf8_chars::{BufReadCharsExt};
use std::iter::Peekable;
use std::io::BufRead;
use std::io::BufReader;
use std::path::PathBuf;
use std::fs::File;
use anyhow::{Context,Result};



#[derive(Debug,PartialEq)]
enum Keyword{
    FN,
    INT,
    DOUBLE,
    VOID,
    CONSUME,
    RETURN,
    TRUE,
    FALSE,
    AND,
    OR,
    NOT
}

#[derive(Debug,PartialEq)]
enum Token{
    KEYWORD(Keyword),
    IDENTIFIER(String),
    VALUE(Value),
    SYMBOL(Symbol),
    OPERATOR(Operator),
    EOF
}

#[derive(Debug,PartialEq)]
enum Symbol {
    LeftCurlyBracket,
    RightCurlyBracket,
    LeftSquareBracket,
    RightSquareBracket,
    LeftParen,
    RightParen,
    COMMA,
    SEMICOLON,
}

#[derive(Debug,PartialEq)]
enum Operator{
    ADD,
    SUB,
    MUL,
    DIV,
    INC,
    DEC,
    ASSIGN,
    GT,
    LT,
    GTE,
    LTE,
    EQ,
    AND,
    OR,
    NOT,
    NEQ
}

#[derive(Debug,PartialEq)]
enum Value {
    INT(i64),
    DOUBLE(f64),
    BOOL(bool),
    STRING(String)
}

#[derive(Error,Debug,PartialEq)]
pub enum Error{
    #[error("Illegal Literal `{0}` at Ln. {1}, Col. {2}")]
    IllegalLiteral(String,u32,u32),
    #[error("Missing `\"` at the end of String Literal `{0}`, at Ln. {1}, Col. {2}")]
    MissingQuote(String,u32,u32),
    #[error("Internal Error, at Ln. {0}, Col. {1}, Please Try Again!")]
    InternalConversionError(u32,u32),
    #[error("Illegal Token `{0}`, at Ln. {1}, Col. {2} ")]
    IllegalToken(char,u32,u32),
    #[error("32 bit Integer Literal Out of Range, at Ln. {0}, Col. {1}")]
    SixtyFourBitIntOutOfRange(String,u32,u32),
    #[error("Internal Error, Please Try again.")]
    InternalError
}

impl Token {
    fn semantical_pack (ky: Keyword) -> Token {
        match ky {
            Keyword::TRUE => Token::VALUE(Value::BOOL(true)),
            Keyword::FALSE => Token::VALUE(Value::BOOL(false)),
            Keyword::AND => Token::OPERATOR(Operator::AND),
            Keyword::OR => Token::OPERATOR(Operator::OR),
            Keyword::NOT => Token::OPERATOR(Operator::NOT),
            _ => Token::KEYWORD(ky)
        }
    }
}

impl Keyword {
    fn is_keyword(name : &str) -> Option<Keyword> {
        match name {
            "fn"  => Some(Keyword::FN),
            "int" => Some(Keyword::INT),
            "double" => Some(Keyword::DOUBLE),
            "void" => Some(Keyword::VOID),
            "consume" => Some(Keyword::CONSUME),
            "true" => Some(Keyword::TRUE),
            "false" => Some(Keyword::FALSE),
            "and" => Some(Keyword::AND),
            "or" => Some(Keyword::OR),
            "not" => Some(Keyword::NOT),
            "return" => Some(Keyword::RETURN),
            _ => None
        }
    }
}

impl Operator {
    fn is_single_operator(ch : char) -> Option<Operator> {
        match ch {
            '+' => Some(Operator::ADD),
            '-' => Some(Operator::SUB),
            '*' => Some(Operator::MUL),
            '/' => Some(Operator::DIV),
            '<' => Some(Operator::LT),
            '>' => Some(Operator::GT),
            '=' => Some(Operator::ASSIGN),
            _ => None
        }
    }

    fn is_multiple_op(str : &str) -> Option<Operator> {
        match str {
            "++" => Some(Operator::INC),
            "--" => Some(Operator::DEC),
            "<=" => Some(Operator::LTE),
            ">=" => Some(Operator::GTE),
            "==" => Some(Operator::EQ),
            "!=" => Some(Operator::NEQ),
            _ => None
        }
    }
}

impl Symbol {
    fn is_symbol(ch : char) -> Option<Symbol> {
        match ch {
            '{' => Some(Symbol::LeftCurlyBracket),
            '}' => Some(Symbol::RightCurlyBracket),
            '[' => Some(Symbol::LeftSquareBracket),
            ']' => Some(Symbol::RightSquareBracket),
            '(' => Some(Symbol::LeftParen),
            ')' => Some(Symbol::RightParen),
            ',' => Some(Symbol::COMMA),
            ';' => Some(Symbol::SEMICOLON),
            _ => None
        }
    }
}


struct DataIterator<T: BufRead + ?Sized>{
    data : T
}

impl<T: BufRead + ?Sized> Iterator for DataIterator<T> {
    type Item = Result<char,std::io::Error>;

    fn next(&mut self) -> Option<Self::Item> {
        self.data.read_char_raw().map_err(|x| x.into_io_error()).transpose()
    }
}

#[allow(dead_code)]
pub struct Lexer<T : std::io::Read>{
    token : Option<Token>,
    row : u32,
    col : u32,
    iter : Peekable<DataIterator<BufReader<T>>>
} 
#[allow(dead_code)]
impl<T: std::io::Read> Lexer<T> {
    pub fn new(name : PathBuf) -> Result<Lexer<File>> {
        let f = File::open(name).with_context(|| format!("Failed to read file"))?;
        let bf = BufReader::new(f);
        let iter = DataIterator{ data : bf };
        let  iter = iter.peekable();
        let lex = Lexer {
            token : None,
            row : 1,
            col : 1,
            iter
        };
        Ok(lex)
    }  
   
    fn test(text : &[u8]) -> Lexer<&[u8]> {
        let bf = BufReader::new(text);
        let iter = DataIterator{data : bf};
        let iter = iter.peekable();
        Lexer {
            token : None,
            row : 1,
            col : 1,
            iter
        }
    }

    fn get_next_char(&mut self) -> Option<char> {
        let token = self.iter.next();
        if let Some(Ok(t)) = token {
            if t == '\n' || t == '\r' {
                self.row += 1;
                self.col = 1;
            }else { self.col += 1; }
            Some(t)
        }else { None }
    }

    fn peek_next_char(&mut self) -> Option<char> {
        let token = self.iter.peek();
        if let Some(Ok(t)) = token { Some(t.clone())}
        else { None }
    }

    fn lex_string(&mut self) -> Result<(),Error> {
        let mut str = String::from("");
        let mut token : Option<char>;
        loop { 
            token = self.get_next_char();
            if let Some(t) = token {
                if t != '"' { str.push(t); }
                else { break; }
            }else { return Err(Error::MissingQuote(str,self.row,self.col))  }
        }
        self.token = Some(Token::VALUE(Value::STRING(str)));
        Ok(())
    }

    fn lex_comment(&mut self)  {
        let mut c : char = '#';
        let mut token : Option<char>;
        while c != '\n' && c != '\r' {
            token = self.get_next_char();
            if let Some(tok) = token { c = tok; }
            else { break; }
        };
    }


    fn lex_alphabetic_seq(&mut self,start : char) ->  Result<(),Error> {
        let mut ch : char = start;
        let mut str = String::from("");
        str.push(ch);
        while ch.is_ascii_alphanumeric() {
            if let Some(c) = self.get_next_char(){ str.push(c);}
            else { return Err(Error::InternalError) }
            if let Some(c) = self.peek_next_char(){ ch  = c; }
            else { break; }
        };
        if let Some(keyword) = Keyword::is_keyword(&str) 
        { self.token = Some(Token::semantical_pack(keyword)); }
        else { self.token = Some(Token::IDENTIFIER(str)); }
        Ok(())
    }


    fn lex_digit(&mut self,start : char) -> Result<(),Error> {
        let mut ch : char = start;
        let mut str = String::from("");
        let mut flag : bool = false;
        let mut lit_err : bool = false;
        let mut counter : u32 = 0;
        str.push(start);
        while ch.is_digit(10) || ch == '.' {
            if let Some(tok) = self.get_next_char(){ ch = tok; }
            str.push(ch);
            counter += 1;
            if ch == '.' {
                if !flag { flag = true; } 
                else { lit_err = true; }
            }
            if let Some(tok) = self.peek_next_char() { ch = tok; }
            else { break; }
        };
        if lit_err { return Err(Error::IllegalLiteral(str,self.row,self.col))  }
        else if flag { 
            if let Ok(number) = str.parse::<f64>(){ self.token = Some(Token::VALUE(Value::DOUBLE(number))); } 
            else {  return Err(Error::InternalConversionError(self.row,self.col))  }
        }
        else { 
            if counter > 39 { return Err(Error::SixtyFourBitIntOutOfRange(str,self.row,self.col))  }
            if let Ok(number) = str.parse::<i64>(){ self.token = Some(Token::VALUE(Value::INT(number))); } 
            else { return Err(Error::InternalConversionError(self.row,self.col)) }
        };
        Ok(())
    }

    fn lex_op_sym(&mut self,start :char) -> Result<(),Error>{
        if let Some(c) = Symbol::is_symbol(start) { 
            self.token = Some(Token::SYMBOL(c)); 

        }
        else {
            let mut op_str = String::new();
            op_str.push(start);
            if let Some(c) = self.peek_next_char() { op_str.push(c); }
            if let Some(c) =  Operator::is_multiple_op(&op_str) {
                self.token =  Some(Token::OPERATOR(c)); 
                self.get_next_char();
            }
            else if let Some(c) = Operator::is_single_operator(start) {  self.token = Some(Token::OPERATOR(c)); }
            else {  return Err(Error::IllegalToken(start,self.row,self.col)) }
        };
        Ok(())
    }

    pub fn get_next_token(&mut self) -> Result<(),Error> {
        let mut token : Option<char> = self.get_next_char();
        loop{
            match token {
                Some(t) if t == '"' => { self.lex_string()?; break; },
                Some(t) if t == '#' => {  
                    self.lex_comment(); 
                    token = self.get_next_char();
                }
                Some(t) if t.is_ascii_whitespace() => { token = self.get_next_char(); },
                Some(t) if t.is_ascii_alphabetic() => { self.lex_alphabetic_seq(t)?; break; },
                Some(t) if t.is_digit(10) => { self.lex_digit(t)?; break; },
                Some(t) if t.is_ascii_punctuation() => { self.lex_op_sym(t)?; break; },
                Some(t) => { return Err(Error::IllegalToken(t,self.row,self.col)) }
                None => { self.token = Some(Token::EOF); break; }
            }
        }
        Ok(())  
    }

    pub fn print_token(&self) {
        match &self.token {
            Some(s) => { println!("Token = {:?}",s); }
            None => {}
        }
    }
}

#[cfg(test)]
mod test{
    use super::*;

    #[test]
    fn test_simple_string() {
        let mut lexer = Lexer::<&[u8]>::test("\"this_is_a_string\"".as_bytes());
        let res = lexer.get_next_token();
        assert_eq!(res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::STRING("this_is_a_string".to_string()))));
    }

    #[test]
    fn test_str_misng_quote_error() {
        let mut lexer = Lexer::<&[u8]>::test("\"".as_bytes());
        let mut _res = lexer.get_next_token();
        assert_eq!(_res,Err(Error::MissingQuote("".to_string(),1,2)));
        assert_eq!(lexer.token,None)
    }

    #[test]
    fn test_comment_lex() {
        let mut lexer = Lexer::<&[u8]>::test("#t#his is a comment@ for test 12342134 \n # this is another comment \r fn".as_bytes());
        let mut _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::FN)));

        let mut lexer =  Lexer::<&[u8]>::test("fn #t#his is a comment@ for test 12342134 \n # this is another comment \r ".as_bytes());
        _res = lexer.get_next_token();
        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::EOF));
    }

    #[test]
    fn test_keyword_lex(){
        let mut lexer = Lexer::<&[u8]>::test("fn return consume int double void ".as_bytes());
        let mut _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::FN)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::RETURN)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::CONSUME)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::INT)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::DOUBLE)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::VOID)));

    }

    #[test]
    fn test_iden_lex() {
        let mut lexer = Lexer::<&[u8]>::test("thisisaniden fn x1 return x2".as_bytes());
        let mut _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::IDENTIFIER("thisisaniden".to_string())));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::FN)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::IDENTIFIER("x1".to_string())));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::KEYWORD(Keyword::RETURN)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::IDENTIFIER("x2".to_string())));
    }


    #[test]
    fn test_values_lex() {
        let mut lexer = Lexer::<&[u8]>::test("10 12312.12313 10.2 12012 true false \"this_is_a_string_value\" ".as_bytes());
        let mut _res = lexer.get_next_token();

        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::INT(10))));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::DOUBLE(12312.12313))));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::DOUBLE(10.2))));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::INT(12012))));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::BOOL(true))));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::BOOL(false))));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::VALUE(Value::STRING("this_is_a_string_value".to_string()))));
    }

    #[test]
    fn test_value_limit_errs() {
        let mut lexer = Lexer::<&[u8]>::test("1231231423125412312312312312312312312312312".as_bytes());
        let mut _res = lexer.get_next_token();
        assert_eq!(_res,Err(Error::SixtyFourBitIntOutOfRange("1231231423125412312312312312312312312312312".to_string(),1,44)));
        assert_eq!(lexer.token,None);
    }

    #[test]
    fn test_operator_lex() {
        let mut lexer = Lexer::<&[u8]>::test("+ - * / = ++ -- and or not >= <= > < == !=".as_bytes());
        let mut _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::ADD)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::SUB)));
        
        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::MUL)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::DIV)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::ASSIGN)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::INC)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::DEC)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::AND)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::OR)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::NOT)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::GTE)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::LTE)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::GT)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::LT)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::EQ)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::OPERATOR(Operator::NEQ)));
    }

    #[test]
    fn test_symbol_lex() {
        let mut lexer = Lexer::<&[u8]>::test("{ ) } , ; ( [ ]".as_bytes());
        let mut _res = lexer.get_next_token();
        
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::LeftCurlyBracket)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::RightParen)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::RightCurlyBracket)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::COMMA)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::SEMICOLON)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::LeftParen)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::LeftSquareBracket)));

        _res = lexer.get_next_token();
        assert_eq!(_res,Ok(()));
        assert_eq!(lexer.token,Some(Token::SYMBOL(Symbol::RightSquareBracket)));
    }

    #[test]
    fn test_unknown_token_err() {
        let mut lexer = Lexer::<&[u8]>::test("^".as_bytes());
        let mut _res = lexer.get_next_token();
        assert_eq!(_res,Err(Error::IllegalToken('^',1,2)));
        assert_eq!(lexer.token,None);
    }
}