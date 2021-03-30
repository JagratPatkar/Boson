use structopt::StructOpt; 
use anyhow::{Context,Result};
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
            _ => None
        }
    }
}

#[derive(Debug)]
enum Token{
    KEYWORD(Keyword),
    IDENTIFIER(String),
    VALUE(Value),
    EOF,
    ERROR(Error)
}

impl Token {
    fn semantical_pack (ky: Keyword) -> Token {
        match ky {
            Keyword::TURE => Token::VALUE(Value::BOOL(true)),
            Keyword::FALSE => Token::VALUE(Value::BOOL(false)),
            _ => Token::KEYWORD(ky)
        }
    }
}

#[derive(Debug)]
enum Value {
    INT(i32),
    DOUBLE(f64),
    BOOL(bool)
}

#[derive(Debug)]
enum Error{
    IlleagalNumber,
    InternalConversionError
}

#[allow(dead_code)]
struct Lexer{
    reader :BufReader<File>,
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
   
    fn get_next_token(&mut self) {
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
                            else { self.token = Some(Token::ERROR(Error::IlleagalNumber)); break 'outer; }
                        }
                        token = char.next();
                        if let Some(it) = token { num.push(it); }
                        else { continue 'outer; }
                        if let Some(c) = char.peek() { ch = c.clone(); }
                        else { break; }
                    }
                    if flag { 
                        if let Ok(number) = num.parse::<f64>(){ self.token = Some(Token::VALUE(Value::DOUBLE(number))); break; }
                        else { self.token = Some(Token::ERROR(Error::InternalConversionError)); break; }
                    }
                    else{
                        if let Ok(number) = num.parse::<i32>(){ self.token = Some(Token::VALUE(Value::INT(number))); break; }
                        else { self.token = Some(Token::ERROR(Error::InternalConversionError)); break; }
                    }  
                },
                Some(_) => {break; },
                None => { self.token = Some(Token::EOF); break; }
            }
        }  
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
    for _i in  1..13 {
        lexer.get_next_token();
        lexer.print_token();
    }
    return Ok(());
}
