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
    CONSUME
}

impl Keyword {
    fn is_keyword(name : &str) -> Option<Keyword> {
        match name {
            "fn"  => Some(Keyword::FN),
            "int" => Some(Keyword::INT),
            "double" => Some(Keyword::DOUBLE),
            "void" => Some(Keyword::VOID),
            "consume" => Some(Keyword::CONSUME),
            _ => None
        }
    }
}

#[derive(Debug)]
enum Token{
    KEYWORD(Keyword),
    IDENTIFIER(String),
    INT(i32),
    EOF,
    ERROR
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
                    { self.token = Some(Token::KEYWORD(keyword)); break; }
                    else { self.token = Some(Token::IDENTIFIER(iden)); break; }
                },
                Some(t) if t.is_digit(10) => {
                    let mut ch : char = t;
                    let mut num : String = String::new();
                    num.push(ch);
                    while ch.is_digit(10) {
                        token = char.next();
                        if let Some(it) = token { num.push(it); }
                        else { continue 'outer; }
                        if let Some(c) = char.peek() { ch = c.clone(); }
                        else { break; }
                    }
                    if let Ok(number) = num.parse::<i32>(){ self.token = Some(Token::INT(number)); break; }
                    else { self.token = Some(Token::ERROR); break; }
                },
                Some(_) => {},
                None => { self.token = Some(Token::EOF); }
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
    lexer.get_next_token();
    lexer.print_token();
    lexer.get_next_token();
    lexer.print_token();
    lexer.get_next_token();
    lexer.print_token();
    lexer.get_next_token();
    lexer.print_token();
    lexer.get_next_token();
    lexer.print_token();
    lexer.get_next_token();
    lexer.print_token();
    return Ok(());
}
