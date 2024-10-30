#pragma once

#include "TokenType.h"

struct Token
{
    TokenType type;
    
    std::string lexeme;
    
    int line;
    int pos;
};
