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

#[allow(dead_code)]
enum Token{
    LeftParen(char),
    RightParen(char),
    Op(char),
    IntNum(i64),
    DoubleNum(f64),
    String(String)
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

   
    // fn getNextToken(&self) -> Option<Token> {
    //     let mut char = self.reader.chars().map(|x| x.unwrap()).peekable();
    //     let char = char.next().unwrap();

    // }

    //Will be used later for testing
    // fn testNew(name : String) -> Lexer<T> {

    // }


}

fn main() -> Result<()> {
    let args = Cli::from_args();
    let content = std::fs::read_to_string(&args.path).with_context(|| format!("could not read file '{:?}'",args.path))?;
    let mut f = File::open(&args.path)?;
    let mut reader = BufReader::new(f);
    let mut char = reader.chars().map(|x| x.unwrap()).peekable();
    println!("{}",char.next().unwrap());
    // let lexer  = Lexer::new(args.path)?;
    
    return Ok(());
}
