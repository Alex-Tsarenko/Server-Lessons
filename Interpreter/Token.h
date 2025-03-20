#pragma once

#include "TokenType.h"
#include "TokenTypeStrings.h"

struct Token
{
    Token( TokenType aType ) : type(aType), lexeme(gTokenTypeStrings[aType]) {}
    Token( TokenType aType, const std::string_view& aLexeme ) : type(aType), lexeme(aLexeme) {}
//    Token( TokenType aType, std::string_view aLexeme, int aLine, int aPos ) : type(aType), lexeme(aLexeme), line(aLine), pos(aPos) {}
//    Token( TokenType aType, std::string_view aLexeme, int aLine, int aPos, int anEndPos )
//                : type(aType), lexeme(aLexeme), line(aLine), pos(aPos), endPos(anEndPos) {}

    TokenType type;
    std::string_view lexeme;
};
