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
    Keyword(Keyword),
    Identifier(String),
    EOF
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
                Some(t) if t.is_whitespace() => { token = char.next(); },
                Some(t) if t.is_alphabetic() => {
                    let mut ch :char = t;
                    let mut iden = String::from("");
                    iden.push(t);
                    while ch.is_alphanumeric(){
                        token = char.next();
                        if let Some(it) = token { iden.push(it); }
                        else { continue 'outer; }
                        if let Some(c) = char.peek(){ ch = c.clone(); }
                        else { break; }
                    }
                    if let Some(keyword) = Keyword::is_keyword(&iden) { self.token = Some(Token::Keyword(keyword)); break; }
                    else { self.token = Some(Token::Identifier(iden)); break; }
                },
                Some(_) => {},
                None => { self.token = Some(Token::EOF); }
            }
        }  
    }
}

fn main() -> Result<()> {
    let args = Cli::from_args();
    let mut lexer = Lexer::new(args.path)?;
    lexer.get_next_token();
    println!("Token One = {:?}",lexer.token.unwrap());
    lexer.get_next_token();
    println!("Token Two = {:?}",lexer.token.unwrap());
    lexer.get_next_token();
    println!("Token Three = {:?}",lexer.token.unwrap());
    return Ok(());
}
