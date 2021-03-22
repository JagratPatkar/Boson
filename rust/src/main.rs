use structopt::StructOpt; 

#[derive(StructOpt)]
struct Cli{
    #[structopt(parse(from_os_str))]
    path : std::path::PathBuf,
}

enum Token{
    LeftParen(char),
    RightParen(char),
    Op(char),
    IntNum(i64),
    DoubleNum(f64),

}

fn main() {
    let args = Cli::from_args();
    let content = std::fs::read_to_string(&args.path).expect("could not read file");
    println!("{}",content);
}
