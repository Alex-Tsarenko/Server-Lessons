#pragma once

enum TokenType
{
    // Keywords
    Class,
    Var,
    Print, Func, Return,
    For, While, Break, Continue,
    If, Elif, Else,
    True, False, None,
    And, Or, Not,
    Pass,
    
    // Operators
    Plus, PlusPlusRight, PlusPlusLeft,
    Minus, MinusMinusRight, MinusMinusLeft,
    Star, DoubleStar,
    Slash, DoubleSlash, Mod,
    Or, Or2, Caret, Ampersand, Tilde,
    LeftShift, RightShift,
    Less, LessEqual,
    Greater, GreaterEqual,
    Assignment, Assignment2, EqualEqual,
    Not, NotEqual,
    PlusEqual, MinusEqual,
    StarEqual, SlashEqual, ModEqual,
    AndEqual, OrEqual, XorEqual,
    LeftShiftEqual,
    RightShiftEqual,
    Dot,
    
    // Punctuators
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    LeftSqBracket, RightSqBracket,
    Comma, Colon, Semicolon,
    Indent, Dedent,
    Newline,
    EndOfFile,
    
    // Identifiers and literals
    Int, Float, Identifier, String,
    
    // Others
    Error,
    EndOfTokenType,
};
