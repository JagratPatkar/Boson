use structopt::StructOpt; 
use anyhow::{Context,Result};
use thiserror::Error;
use utf8_chars::{BufReadCharsExt};
use std::fs::File;
use std::io::BufReader;
use std::path::PathBuf;

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
    BOOL(bool)
}

#[derive(Error,Debug)]
enum Error{
    #[error("Illegal Number used")]
    IllegalNumber,
    #[error("Internal Error, Please Try Again!")]
    InternalConversionError,
    #[error("Illegal Token `{0}` ")]
    IllegalToken(char)
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
    token : Option<Token>
}
impl Lexer
{
    fn new(name : PathBuf) -> Result<Lexer> {
        let f = File::open(name).with_context(|| format!("Failed to read file"))?;
        Ok(Lexer {
            reader : BufReader::new(f),
            token : None
        })
    }  
   
    fn get_next_token(&mut self) -> Result<(),Error> {
        let mut char = self.reader.chars().map(|x| x.unwrap()).peekable();
        let mut token : Option<char> = Some(' ');
        'outer:loop{
            match token {
                Some(t) if t.is_ascii_whitespace() => { token = char.next(); },
                Some(t) if t.is_ascii_alphabetic() => {
                    let mut ch :char = t;
                    let mut iden = String::from("");
                    iden.push(t);
                    while ch.is_ascii_alphanumeric(){
                        token = char.next();
                        if let Some(it) = token { iden.push(it); }
                        else { continue 'outer; }
                        if let Some(c) = char.peek(){ ch = c.clone(); }
                        else { break; }
                    }
                    if let Some(keyword) = Keyword::is_keyword(&iden) 
                    { self.token = Some(Token::semantical_pack(keyword)); break; }
                    else { self.token = Some(Token::IDENTIFIER(iden)); break; }
                },
                Some(t) if t.is_digit(10) => {
                    let mut ch : char = t;
                    let mut num : String = String::new();
                    let mut flag : bool = false;
                    num.push(ch);
                    while ch.is_digit(10) || ch == '.' {
                        if ch == '.' {
                            if !flag { flag = true;} 
                            else { return Err(Error::IllegalNumber) }
                        }
                        token = char.next();
                        if let Some(it) = token { num.push(it); }
                        else { continue 'outer; }
                        if let Some(c) = char.peek() { ch = c.clone(); }
                        else { break; }
                    }
                    if flag { 
                        if let Ok(number) = num.parse::<f64>(){ self.token = Some(Token::VALUE(Value::DOUBLE(number))); break; }
                        else { return Err(Error::InternalConversionError) }
                    }
                    else{
                        if let Ok(number) = num.parse::<i32>(){ self.token = Some(Token::VALUE(Value::INT(number))); break; }
                        else { return Err(Error::InternalConversionError) }
                    }  
                },
                Some(t) if t.is_ascii_punctuation() => {
                    if let Some(c) = Symbol::is_symbol(t) { self.token = Some(Token::SYMBOL(c)); break; }
                    else {
                        let mut op_str = String::new();
                        op_str.push(t);
                        if let Some(c) = char.peek() { op_str.push(c.clone()); }
                        if let Some(c) = Operator::is_multiple_op(&op_str)  { self.token = Some(Token::OPERATOR(c)); break; }
                        else if let Some(c) = Operator::is_single_operator(t) { self.token = Some(Token::OPERATOR(c)); break; }
                        else { return Err(Error::IllegalToken(t)) }
                    }
                },
                Some(t) => return Err(Error::IllegalToken(t)),
                None => { self.token = Some(Token::EOF); break; }
            }
        }
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
    for _i in  1..37 {
        lexer.get_next_token()?;
        lexer.print_token();
    }
    return Ok(());
}
