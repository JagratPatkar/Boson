use structopt::StructOpt; 
use anyhow::{Context,Result};
use utf8_chars::{BufReadCharsExt};
use utf8_chars::Chars;
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
    Keyword(Keyword),
    Identifier(String)
}

#[allow(dead_code)]
struct Lexer{
    reader :BufReader<File>,
    token : char
}

impl Lexer
{
    fn new(name : PathBuf) -> Result<Lexer> {
        let f = File::open(name).with_context(|| format!("Failed to read file"))?;
        Ok(Lexer {
            reader : BufReader::new(f),
            token : ' '
        })
    }  

   
    fn get_next_token(&mut self) -> Option<Token> {
        let mut char = self.reader.chars().map(|x| x.unwrap()).peekable();
        let mut token : char = ' ';
        match char.next() {
            Some(tok) => {
                token = tok;
            },
            _ => {}
        }
       
        while token.is_whitespace() {
            token = char.next().unwrap();
        };

        if token.is_alphabetic() {
            let mut iden = String::from("");
            iden.push(token);

            while token.is_alphanumeric() {
                token = char.next().unwrap();
                iden.push(token);
                token = char.peek().unwrap().clone();
            }
            if let Some(keyword) = Keyword::is_keyword(&iden) {
                return Some(Token::Keyword(keyword));
            }else {
                return Some(Token::Identifier(iden));
            };
        }
        unreachable!();
    }
}

fn main() -> Result<()> {
    let args = Cli::from_args();
    let mut lexer = Lexer::new(args.path)?;
    println!("Token One = {:?}",lexer.get_next_token().unwrap());
    println!("Token Two = {:?}",lexer.get_next_token().unwrap());
    println!("Token Three = {:?}",lexer.get_next_token().unwrap());
    return Ok(());
}
