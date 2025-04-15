#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include "Logs.h"
#include "Token.h"
#include "TokenTypeStrings.h"
#include <cassert>

#define IGNORE_COMMENTS 1

// Scanner
class Lexer
{
    const char* m_ptr;
    const char* m_endPtr;

    std::vector<Token> m_tokens;
    
    int m_isBlockLevel = 0;
    
public:
    Lexer( const char* begin, const char* end ) : m_ptr(begin), m_endPtr(end) {}

    const std::vector<Token>& tokens() const { return m_tokens; }

    void addToken( TokenType type, const std::string_view& lexeme )
    {
        LOG( "add token: '" << gTokenTypeStrings[type] << "'  " << lexeme );
        m_tokens.push_back( Token{ type, lexeme } );
    }
    
    void addToken( Token&& token )
    {
        LOG( "add token: " << gTokenTypeStrings[token.type] << " " << token.lexeme );
        m_tokens.push_back( std::move(token) );
    }
    
    bool ifNext( char c )
    {
        if ( m_ptr+1 >= m_endPtr )
        {
            return false;
        }
        if ( *(m_ptr+1) == c )
        {
            m_ptr++;
            return true;
        }
        return false;
    }

    void handleStringLiteral( char delimiter )
    {
        auto start = m_ptr;
        m_ptr++;
        int slashCounter = 0;
        while( (m_ptr < m_endPtr) && (*m_ptr != delimiter) )
        {
            if ( *m_ptr == '\\' )
            {
                m_ptr++;
                switch( *m_ptr )
                {
                    case '"':
                    case '\'':
                    case '\\':
                    case 'n':
                    case 'r':
                    case 't':
                    case '0':
                        slashCounter++;
                        break;
                    case 'x':
                        slashCounter+=3;
                        break;

                }
                if ( m_ptr == m_endPtr )
                {
                    throw std::runtime_error("Unexpected end of file." );
                }
            }
            if ( *m_ptr == '$' )
            {
                m_ptr++;
                switch( *m_ptr )
                {
                    case '"':
                    case '\'':
                    case '\\':
                    case 'n':
                    case 'r':
                    case 't':
                    case '0':
                        slashCounter++;
                        break;
                    case 'x':
                        slashCounter+=3;
                        break;

                }
                if ( m_ptr == m_endPtr )
                {
                    throw std::runtime_error("Unexpected end of file." );
                }
            }
            m_ptr++;
        }

        addToken( Token{ TokenType::StringLiteral, std::string_view(start+1,m_ptr) } );

//        if ( slashCounter == 0 )
//        {
//            addToken( Token{ TokenType::StringLiteral, std::string_view(start+1,m_ptr) } );
//        }
//        else
//        {
//            std::string literal;
//            literal.reserve( m_ptr-start+1-slashCounter );
//            
//            for( auto ptr=start+1; ptr<m_ptr; ptr++ )
//            {
//                if ( *ptr == '\\')
//                {
//                    switch( *(ptr+1) )
//                    {
//                        case '"':
//                        case '\'':
//                        case '\\':
//                            literal.append(1,*ptr);
//                            break;
//                        case 'n':
//                            literal.append(1,'\n');
//                            break;
//                        case 'r':
//                            literal.append(1,'\r');
//                            break;
//                        case 't':
//                            literal.append(1,'\t');
//                            break;
//                        case '0':
//                            literal.append(1,0);
//                            break;
//                        case 'x':
//                        {
//                            ptr++;
//                            uint8_t c = std::stoi( std::string(ptr,ptr+2), nullptr, 16 );
//                            literal.append(1,c);
//                            ptr++;
//                            break;
//                        }
//                        default:
//                            literal.append(1,*ptr);
//                            literal.append(1,*(ptr+1));
//                            break;
//                    }
//                    ptr++;
//                }
//                else
//                {
//                    literal.append(1,*ptr);
//                }
//            }
//            addToken( Token{ TokenType::StringLiteral, std::move(literal), m_line, startPos, m_pos } );
//        }
        
        LOG( "TokenType::String: <" << m_tokens.back().lexeme << ">")
    }
    
    void handleNumber( )
    {
        //99.01.01
        /*
         *    number ::= integer | floatnumber
         *    integer ::= decimalinteger
         *                | octinteger | hexinteger  // TODO
        */
        auto start = m_ptr;
        m_ptr++;
        while( (m_ptr < m_endPtr) && isdigit(*m_ptr) )
        {
            m_ptr++;
        }
        
        bool hasDecimal = false;
        if ( *m_ptr == '.')
        {
            hasDecimal = true;
            m_ptr++;
            while( (m_ptr < m_endPtr) && isdigit(*m_ptr) )
            {
                m_ptr++;
            }
        }
        addToken( Token{ hasDecimal ? TokenType::Float : TokenType::Int, std::string_view{start,m_ptr} } );
    }
    
    std::string tolower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                    // static_cast<int(*)(int)>(std::tolower)         // wrong
                    // [](int c){ return std::tolower(c); }           // wrong
                    // [](char c){ return std::tolower(c); }          // wrong
                       [](unsigned char c){ return std::tolower(c); } // correct
                      );
        return s;
    }
    
    TokenType typeOf( const std::string_view& lexeme )
    {
        //        Class,
        //        Print, Func, Return,
        //        While, Break, Continue,
        //        If, Elif, Else,
        //        True, False, None,
        //        And, Or, Not,
        //        Pass,
        
        static std::unordered_map<std::string_view,TokenType> map = {
            {"namespace",NamespaceKw},
            {"class",ClassKw},
            {"private",Private},
            {"var",Var},
            {"if",If},
            {"for",For},
            {"return",Return},
            {"print",Print},
            {"func",Func},
            {"private",Private},
            {"print",Print},
//            {"int",Int},
//            {"float",Float},
//            {"string",String},
            {"string-literal",StringLiteral},
        };
        
        auto it = map.find( lexeme );
        if ( it == map.end() )
        {
            return Identifier;
        }
        return it->second;
    }
    
    void handleIdentifier( )
    {
        /*
         *   identifier ::= (letter | "_") (letter | digit | "_")*
         *   letter ::= lowercase | uppercase
         *   lowercase ::= "a" | "b" | ... | "z"
         *   uppercase ::= "A" | "B" | ... | "Z"
         *   digit ::= "0" | "1" | ... | "9"
         *
         *  (where the matched string is not a keyword)
        */
        auto start = m_ptr;
        m_ptr++;
        while( (m_ptr < m_endPtr) && (isalnum(*m_ptr) or *m_ptr == '_') )
        {
            m_ptr++;
        }

        auto lexeme = std::string_view{ start, m_ptr };

        TokenType type = typeOf( lexeme );

        // Check if the identifier indicates the start of a block
        switch(type) {
            case TokenType::If:
            case TokenType::Elif:
            case TokenType::Else:
            case TokenType::While:
            case TokenType::Func:
            case TokenType::ClassKw: {
                m_isBlockLevel++;
                break;
            }
            default: break;
        }

        if ( type == TokenType::Identifier )
        {
            if ( m_tokens.back().type == IdentifierWithScope )
            {
                m_tokens.back().lexeme = std::string_view{ m_tokens.back().lexeme.data(), m_ptr };
            }
            else
            {
                addToken( Token{ TokenType::Identifier, lexeme } );
            }
        }
        else
        {
            addToken( Token{ type, lexeme } );
        }

        m_ptr--;
        assert( *m_ptr != '\n' );
    }

    void run()
    {
        addToken( TokenType::Begin, "<begin>" );

        for(; m_ptr < m_endPtr; m_ptr++ )
        {
            switch( *m_ptr )
            {
                case 0:
                    break;
                case ':':
                    if ( ifNext(':') )
                    {
                        if ( m_tokens.back().type == Identifier || m_tokens.back().type == IdentifierWithScope )
                        {
                            m_tokens.back().type = IdentifierWithScope;
                            m_tokens.back().lexeme = std::string_view{ m_tokens.back().lexeme.data(), m_ptr+1 };
                        }
                        else
                        {
                            addToken( TokenType::IdentifierWithScope, {m_ptr-1,2} );
                        }
                        break;
                    }
                    addToken( TokenType::Colon, {m_ptr,1} );
                    break;
                case '(':
                    addToken( TokenType::LeftParen, {m_ptr,1} );
                    break;
                case ')':
                    addToken( TokenType::RightParen, {m_ptr,1} );
                    break;
                case '{':
                    addToken( TokenType::LeftBrace, {m_ptr,1} );
                    m_isBlockLevel++;
                    break;
                case '}':
                    addToken( TokenType::RightBrace, {m_ptr,1} );
                    m_isBlockLevel--;
                    break;
                case '[':
                    addToken( TokenType::LeftSqBracket, {m_ptr,1} );
                    m_isBlockLevel++;
                    break;
                case ']':
                    addToken( TokenType::RightSqBracket, {m_ptr,1} );
                    m_isBlockLevel--;
                    break;
                case ',':
                    addToken( TokenType::Comma, {m_ptr,1} );
                    break;
                case '.':
//                    if ( m_tokens.back().type == Identifier || m_tokens.back().type == IdentifierWithScope )
//                    {
//                        m_tokens.back().type = IdentifierWithScope;
//                        m_tokens.back().lexeme = std::string_view{ m_tokens.back().lexeme.data(), m_ptr+1 };
//                        break;
//                    }

                    addToken( TokenType::Dot, {m_ptr,1} );
                    break;
                case '-':
                    if ( ifNext('=') )
                    {
                        addToken( TokenType::MinusEqual, {m_ptr-1,2} );
                    }
                    else if ( ifNext('-') )
                    {
                        auto size = m_tokens.size();
                        if ( size > 0 && m_tokens[size-1].type == TokenType::Identifier )
                        {
                            addToken( TokenType::MinusMinusRight, {m_ptr-1,2} );
                        }
                        else
                        {
                            addToken( TokenType::MinusMinusLeft, {m_ptr-1,2} );
                        }
                    }
                    else
                    {
                        addToken( TokenType::Minus, {m_ptr,1} );
                    }
                    break;
                case '+':
                    if ( ifNext('=') )
                    {
                        addToken( TokenType::PlusEqual, {m_ptr,1} );
                    }
                    else if ( ifNext('+') )
                    {
                        auto size = m_tokens.size();
                        if ( size > 0 && m_tokens[size-1].type == TokenType::Identifier )
                        {
                            addToken( TokenType::PlusPlusRight, {m_ptr-1,2} );
                        }
                        else
                        {
                            addToken( TokenType::PlusPlusLeft, {m_ptr-1,2} );
                        }
                    }
                    else
                    {
                        addToken( TokenType::Plus, {m_ptr,1} );
                    }
                    break;
                case ';':
                    addToken( TokenType::Semicolon, {m_ptr,1} );
                    break;
                case '*':
                    if ( ifNext('*') )
                    {
                        addToken(  TokenType::DoubleStar, {m_ptr-1,2} );
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::StarEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Star, {m_ptr,1} );
                    }
                    break;
                case '/':
                    if ( ifNext('/') )
                    {
                        auto start = m_ptr-1;
                        while( *m_ptr != '\n' && *m_ptr != '\r' ) m_ptr++;

                        #ifndef IGNORE_COMMENTS
                            addToken( Token{ TokenType::Comment, std::string_view(start,m_ptr) } );
                        #endif
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::SlashEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Slash, {m_ptr,1} );
                    }
                    break;
                case '%':
                    if ( ifNext('=') )
                    {
                        addToken(  TokenType::ModEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Mod, {m_ptr,1} );
                    }
                    break;
                case '|':
                    addToken( ifNext('=') ? TokenType::OrEqual :
                             ifNext('|') ? TokenType::Or : TokenType::BitOr, "<todo|>" );
                    break;
                case '&':
                    addToken( ifNext('=') ? TokenType::AndEqual :
                             ifNext('&') ? TokenType::And : TokenType::BitAnd, "<todo&>" );
                    break;
                case '^':
                    if ( ifNext('=') )
                    {
                        addToken(  TokenType::XorEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Xor, {m_ptr,1} );
                    }
                    break;
                case '~':
                    addToken( TokenType::Tilde, {m_ptr,1} );
                    break;
                case '!':
                    if ( ifNext('=') )
                    {
                        addToken(  TokenType::NotEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Not, {m_ptr,1} );
                    }
                    break;
                case '=':
                    if ( ifNext('=') )
                    {
                        addToken(  TokenType::EqualEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Assignment, {m_ptr,1} );
                    }
                    break;
                case '<':
                    if ( ifNext('<') )
                    {
                        if ( ifNext('=') )
                        {
                            addToken( TokenType::LeftShiftEqual, {m_ptr-2,3} );
                        }
                        else
                        {
                            addToken(  TokenType::LeftShift, {m_ptr-1,2} );
                        }
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::LessEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Less, {m_ptr,1} );
                    }
                    break;
                case '>':
                    if ( ifNext('>') )
                    {
                        if ( ifNext('=') )
                        {
                            addToken( TokenType::RightShiftEqual, {m_ptr-2,3} );
                        }
                        else
                        {
                            addToken(  TokenType::RightShift, {m_ptr-1,2} );
                        }
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::GreaterEqual, {m_ptr-1,2} );
                    }
                    else
                    {
                        addToken( TokenType::Greater, {m_ptr,1} );
                    }
                    break;
//                 case '#':
//                    for( m_ptr++; (m_ptr < m_endPtr) && (*m_ptr != '\n'); m_ptr++ )
//                    {
//                    }
//                    m_ptr--;
//                    m_line++;
//                    m_pos=-1;
//                    break;
                case ' ':
                case '\t':
                case '\r':
                    // ignore whitespaces
                    break;
                case '\n':
                    //addToken( TokenType::Newline, "\\n" );
                    break;

                    // string literals
                case '\"':
                case '\'':
                    handleStringLiteral( *m_ptr );
                    break;
                    
                default:
                    if ( isdigit( *m_ptr ) ) {
                        handleNumber( );
                    } 
                    else if ( isalpha(*m_ptr) or *m_ptr == '_')
                    {
                        handleIdentifier( );
                    } 
                    else
                    {
                        throw std::runtime_error("Unexpected character." );
                    }
                    break;
            }
        }
        addToken( TokenType::EndOfFile, "<eof>" );
    }
};
