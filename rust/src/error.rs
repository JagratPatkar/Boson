use thiserror::Error;

#[derive(Error,Debug,PartialEq)]
pub enum LexError{
    #[error("Illegal Literal `{0}` at Ln. {1}, Col. {2}")]
    IllegalLiteral(String,u32,u32),
    #[error("Missing `\"` at the end of String Literal `{0}`, at Ln. {1}, Col. {2}")]
    MissingQuote(String,u32,u32),
    #[error("Internal Error, at Ln. {0}, Col. {1}, Please Try Again!")]
    InternalConversionError(u32,u32),
    #[error("Illegal Token `{0}`, at Ln. {1}, Col. {2} ")]
    IllegalToken(char,u32,u32),
    #[error("32 bit Integer Literal Out of Range, at Ln. {0}, Col. {1}")]
    SixtyFourBitIntOutOfRange(String,u32,u32),
    #[error("Internal Error, Please Try again.")]
    InternalError
}