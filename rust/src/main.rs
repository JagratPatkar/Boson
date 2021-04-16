use structopt::StructOpt; 
use anyhow::{Context,Result};
use thiserror::Error;
use utf8_chars::{BufReadCharsExt};
use std::iter::Peekable;
use std::fs::File;



#[derive(StructOpt)]
struct Cli{
    #[structopt(parse(from_os_str))]
    path : std::path::PathBuf,
}



fn main() -> Result<()> {
    let args = Cli::from_args();
    // let mut lexer = Lexer::<File>::new(args.path)?;
    // for _i in  1..39 {
    //     lexer.get_next_token()?;
    //     lexer.print_token();
    // }
    return Ok(());
}
