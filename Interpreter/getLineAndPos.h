#pragma once

#include <string>
#include "Token.h"

inline void getLineAndPos( const std::string_view text, const Token& token, int& line, int& pos, int& endPos )
{
    for( const char* p = text.data(); p < text.data() + text.size(); ++p )
    {
        if( *p == '\n' )
        {
            line++;
            pos = 0;
            if( *(p+1) == '\r' )
            {
                ++p;
            }
            continue;
        }
        else if( *p == '\r' )
        {
            line++;
            pos = 0;
            if( *(p+1) == '\n' )
            {
                ++p;
            }
            continue;
        }

        pos++;

        if( p == token.lexeme.data() )
        {
            endPos = pos + int(token.lexeme.size());
            return;
        }
    }
}
