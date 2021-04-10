use structopt::StructOpt; 
use anyhow::{Context,Result};
use thiserror::Error;
use utf8_chars::{BufReadCharsExt,Chars};
use std::iter::Peekable;
use std::vec::Vec;
use std::fs::File;
use std::io::BufReader;
use std::path::PathBuf;
use std::slice::Iter;

#[derive(StructOpt)]
struct Cli{
    #[structopt(parse(from_os_str))]
    path : std::path::PathBuf,
}

#[derive(Debug)]
enum Keyword{
    FN,
    INT,
    DOUBLE,
    VOID,
    CONSUME,
    TURE,
    FALSE,
    AND,
    OR,
    NOT
}

#[derive(Debug)]
enum Token{
    KEYWORD(Keyword),
    IDENTIFIER(String),
    VALUE(Value),
    SYMBOL(Symbol),
    OPERATOR(Operator),
    EOF
}

#[derive(Debug)]
enum Symbol {
    LeftCurlyBracket,
    RightCurlyBracket,
    LeftSquareBracket,
    RightSquareBracket,
    LeftParen,
    RghtParen,
    COMMA,
    SEMICOLON,
}

#[derive(Debug)]
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

#[derive(Debug)]
enum Value {
    INT(i32),
    DOUBLE(f64),
    BOOL(bool),
    STRING(String)
}

#[derive(Error,Debug)]
enum Error{
    #[error("Illegal Literal `{0}` at line no. {1} col no. {2}")]
    IllegalLiteral(String,u32,u32),
    #[error("Missing `\"` at the end of String Literal `{0}` at line no. {1} col no. {2}")]
    MissingQuote(String,u32,u32),
    #[error("Internal Error, at line no. {0} col no. {1}, Please Try Again!")]
    InternalConversionError(u32,u32),
    #[error("Illegal Token `{0}` at line no. {1} col no. {2} ")]
    IllegalToken(char,u32,u32)
}

impl Token {
    fn semantical_pack (ky: Keyword) -> Token {
        match ky {
            Keyword::TURE => Token::VALUE(Value::BOOL(true)),
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
            "true" => Some(Keyword::TURE),
            "false" => Some(Keyword::FALSE),
            "and" => Some(Keyword::AND),
            "or" => Some(Keyword::OR),
            "not" => Some(Keyword::NOT),
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
            ')' => Some(Symbol::RghtParen),
            ',' => Some(Symbol::COMMA),
            ';' => Some(Symbol::SEMICOLON),
            _ => None
        }
    }
}


#[allow(dead_code)]
struct Lexer{
    reader : BufReader<File>,
    token : Option<Token>,
    row : u32,
    col : u32,
    peekd_char : Option<char> 
} 


impl Lexer
{
    fn new(name : PathBuf) -> Result<Lexer> {
        let f = File::open(name).with_context(|| format!("Failed to read file"))?;
        Ok(Lexer {
            reader :  BufReader::new(f),
            token : None,
            row : 1,
            col : 1,
            peekd_char : None
        })
    }  
     

    fn trsnf_data(&mut self,row :u32,col : u32){
        self.row = row;
        self.col = col;
    }

    fn get_next_token(&mut self) -> Result<(),Error> {
        let mut row : u32 = self.row;
        let mut col : u32 = self.col;
        let mut ch : Option<char> = None;
        let mut char = self.reader.chars().peekable();
        let mut get_next_char = |flg| {
            if flg {  
                let token = char.next();
                if let Some(Ok(t)) = token {
                    if t == '\n' || t == '\r' { 
                        row += 1; 
                        col = 0;
                    }
                    else { col += 1; }
                    Some(t)
                }else { None }
            }else {
                let token = char.peek();
                if let Some(Ok(t)) = token { Some(t.clone()) }
                else { None }
            }
        };
        let mut token : Option<char>;
        if let Some(t) = self.peekd_char { 
            token = Some(t); 
            self.peekd_char = None; 
        }
        else { token = get_next_char(true); }
        'outer:loop{
            match token {
                Some(t) if t == '"' => {
                    let mut str = String::from("");
                    loop {
                        token = get_next_char(true);
                        if let Some(tok) = token {
                            if tok != '"' { str.push(tok); }
                            else { break; }
                        }                           
                        else { 
                            self.trsnf_data(row, col);
                            return Err(Error::MissingQuote(str,row,col)) 
                        }
                    }
                    self.token = Some(Token::VALUE(Value::STRING(str)));
                    break;
                },
                Some(t) if t == '#' => {
                    let mut c : char = t;
                    while c != '\n' && c != '\r' {
                        token =  get_next_char(true);
                        if let Some(tok) = token { c = tok; }
                        else { break; }
                    } 
                }
                Some(t) if t.is_ascii_whitespace() => { token =  get_next_char(true); },
                Some(t) if t.is_ascii_alphabetic() => {
                    let mut ch :char = t;
                    let mut iden = String::from("");
                    iden.push(t);
                    loop {
                        if let Some(c) = get_next_char(false){ ch = c; }
                        else { break; }
                        if ch.is_ascii_alphanumeric(){
                            token = get_next_char(true);
                            if let Some(it) = token {  iden.push(it); }
                            else { continue 'outer; }
                        }
                        else { self.peekd_char = Some(ch); break; }
                    }
                    if let Some(keyword) = Keyword::is_keyword(&iden) 
                    { self.token = Some(Token::semantical_pack(keyword)); break; }
                    else { self.token = Some(Token::IDENTIFIER(iden)); break; }
                },
                Some(t) if t.is_digit(10) => {
                    let mut ch : char = t;
                    let mut num : String = String::new();
                    let mut flag : bool = false;
                    let mut lit_err : bool = false;
                    num.push(ch);
                    while ch.is_digit(10) || ch == '.' {
                        if ch == '.' {
                            if !flag { flag = true; } 
                            else { lit_err = true; }
                        }
                        token = get_next_char(true);
                        if let Some(it) = token { num.push(it); }
                        else { continue 'outer; }
                        if let Some(c) = get_next_char(false) { ch = c; }
                        else { break; }
                    }
                    self.peekd_char = Some(ch);
                    if lit_err {
                        self.trsnf_data(row, col); 
                        return Err(Error::IllegalLiteral(num,row,col)) 
                    }
                    if flag { 
                        if let Ok(number) = num.parse::<f64>(){ self.token = Some(Token::VALUE(Value::DOUBLE(number))); break; }
                        else { 
                            self.trsnf_data(row, col);
                            return Err(Error::InternalConversionError(row,col)) 
                        }
                    }
                    else{
                        if let Ok(number) = num.parse::<i32>(){ self.token = Some(Token::VALUE(Value::INT(number))); break; }
                        else { 
                            self.trsnf_data(row, col);
                            return Err(Error::InternalConversionError(row,col)) 
                        }
                    }  
                },
                Some(t) if t.is_ascii_punctuation() => {
                    if let Some(c) = Symbol::is_symbol(t) { self.token = Some(Token::SYMBOL(c)); break; }
                    else {
                        let mut op_str = String::new();
                        let mut sec_ch : Option<char> = None;
                        op_str.push(t);
                        if let Some(c) = get_next_char(false) { 
                            sec_ch = Some(c); 
                            op_str.push(c); 
                        }
                        if let Some(c) = Operator::is_multiple_op(&op_str)  { 
                            self.token = Some(Token::OPERATOR(c));
                            get_next_char(true);
                            break; 
                        }
                        else if let Some(c) = Operator::is_single_operator(t) { 
                            self.token = Some(Token::OPERATOR(c));
                            break; 
                        }
                        else { 
                            self.peekd_char =sec_ch;
                            self.trsnf_data(row, col);
                            return Err(Error::IllegalToken(t,row,col)) 
                        }
                    }
                },
                Some(t) => {
                    self.trsnf_data(row, col);
                    return Err(Error::IllegalToken(t,row,col))
                }
                None => { self.token = Some(Token::EOF); break; }
            }
        }
        self.trsnf_data(row, col);
        Ok(())  
    }

    fn print_token(&self) {
        match &self.token {
            Some(s) => { println!("Token = {:?}",s); }
            None => {}
        }
    }
}


fn main() -> Result<()> {
    let args = Cli::from_args();
    let mut lexer = Lexer::new(args.path)?;
    for _i in  1..39 {
        lexer.get_next_token()?;
        lexer.print_token();
    }
    return Ok(());
}
