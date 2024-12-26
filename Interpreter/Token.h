#pragma once

#include "TokenType.h"
#include "TokenTypeStrings.h"

struct Token
{
    Token( TokenType aType ) : type(aType), lexeme(gTokenTypeStrings[aType]) {}
    Token( TokenType aType, std::string aLexeme ) : type(aType), lexeme(aLexeme) {}
    Token( TokenType aType, std::string aLexeme, int aLine, int aPos ) : type(aType), lexeme(aLexeme), line(aLine), pos(aPos) {}
    Token( TokenType aType, std::string aLexeme, int aLine, int aPos, int anEndPos ) : type(aType), lexeme(aLexeme), line(aLine), pos(aPos), endPos(anEndPos) {}

    TokenType type;
    std::string lexeme;
    
    int line;
    int pos;
    int endPos;
};
