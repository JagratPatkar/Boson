use structopt::StructOpt; 
mod lexer;
use lexer::Lexer;
use std::fs::File;
use anyhow::{Result};



#[derive(StructOpt)]
struct Cli{
    #[structopt(parse(from_os_str))]
    path : std::path::PathBuf,
}



fn main() -> Result<()> {
    let args = Cli::from_args();
    let mut lexer = Lexer::<File>::new(args.path)?;
    for _i in  1..39 {
        lexer.get_next_token()?;
        lexer.print_token();
    }
    return Ok(());
}
