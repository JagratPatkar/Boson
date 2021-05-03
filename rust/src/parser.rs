
use super::lexer::*;
use super::error::Errors;
use std::fs::File;

struct Parser {
    lexer : Lexer<File>
}

impl Parser {

    fn handler(&mut self) -> Result<(),Errors> {
        loop {
            self.lexer.get_next_token();
            if self.lexer.is_token_valid_type() { self.parse_variable_dec()?; }
            else if self.lexer.is_token_eof() { break; }
        };
        Ok(())
    }

    fn parse_variable_dec(&mut self) -> Result<(),Errors> {
        self.lexer.get_next_token();
        let  iden : String;
        if self.lexer.is_token_iden() {
            iden = self.lexer.get_iden().unwrap();
            // Create an Identifire AST Node
            // Lex the Assign Symbol
        }
        Ok(())
    }
}