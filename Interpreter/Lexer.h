#pragma once

#include <vector>
#include <map>
#include "Logs.h"
#include "Token.h"
#include "TokenTypeStrings.h"

// Scanner
class Lexer
{
    const char* m_ptr;
    const char* m_endPtr;
    int         m_line = 0;
    int         m_pos  = 0;

    std::vector<Token> m_tokens;
    
    int m_isBlockLevel = 0;
    
public:
    Lexer( const char* begin, const char* end ) : m_ptr(begin), m_endPtr(end) {}

    const std::vector<Token>& tokens() const { return m_tokens; }

    void addToken( TokenType type, const std::string& lexeme )
    {
        m_tokens.push_back( Token{ type, lexeme, m_line, m_pos } );
    }
    
    void addToken( Token&& token )
    {
        LOG( "token: " << gTokenTypeStrings[token.type] << " " << token.lexeme );
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

    void handleIndentation( )
    {
        
    }
    
    void handleStringLiteral( char delimiter )
    {
        auto start = m_ptr;
        auto startPos = m_pos;
        m_ptr++;
        m_pos++;
        while( (m_ptr < m_endPtr) && (*m_ptr != delimiter) )
        {
            if ( *m_ptr == '\\' )
            {
                m_ptr++;
                m_pos++;
                if ( m_ptr == m_endPtr )
                {
                    throw std::runtime_error("Unexpected end of file." );
                }
            }
            m_ptr++;
            m_pos++;
        }
        
        addToken( Token{ TokenType::String, std::string(start+1,m_ptr), m_line, startPos, m_pos } );
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
        auto startPos = m_pos;
        m_ptr++;
        m_pos++;
        while( (m_ptr < m_endPtr) && isdigit(*m_ptr) )
        {
            m_ptr++;
            m_pos++;
        }
        
        bool hasDecimal = false;
        if ( *m_ptr == '.')
        {
            hasDecimal = true;
            m_ptr++;
            m_pos++;
            while( (m_ptr < m_endPtr) && isdigit(*m_ptr) )
            {
                m_ptr++;
                m_pos++;
            }
        }
        addToken( Token{ hasDecimal ? TokenType::Float : TokenType::Int, std::string(start,m_ptr), m_line, startPos, m_pos } );
        m_ptr--;
        m_pos--;
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
    
    TokenType typeOf( const std::string& lexeme )
    {
        //        Class,
        //        Print, Func, Return,
        //        While, Break, Continue,
        //        If, Elif, Else,
        //        True, False, None,
        //        And, Or, Not,
        //        Pass,
        
        static std::unordered_map<std::string,TokenType> map = {
            {"var",Var},
            {"if",If},
            {"for",For},
            {"return",Return},
            {"print",Print},
            {"func",Func},
            {"class",Class},
            {"print",Print},
            {"int",Int},
            {"float",Float},
            {"string",String},
        };
        
        auto it = map.find( tolower(lexeme) );
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
        auto startPos = m_pos;
        m_ptr++;
        m_pos++;
        while( (m_ptr < m_endPtr) && (isalnum(*m_ptr) or *m_ptr == '_') )
        {
            m_ptr++;
            m_pos++;
        }

        auto lexeme = std::string(start,m_ptr);

        TokenType type = typeOf( lexeme );

        // Check if the identifier indicates the start of a block
        switch(type) {
            case TokenType::If:
            case TokenType::Elif:
            case TokenType::Else:
            case TokenType::While:
            case TokenType::Func:
            case TokenType::Class: {
                m_isBlockLevel++;
                break;
            }
            default: break;
        }

        addToken( Token{ type, std::move(lexeme), m_line, startPos, m_pos } );

        m_ptr--;
        m_pos--;
    }

    void run()
    {
        addToken( TokenType::Begin, "<begin>" );

        while( m_ptr < m_endPtr )
        {
            m_ptr++;
            m_pos++;
            
            switch( *m_ptr )
            {
                case 0:
                    break;
                case ':':
                    addToken( TokenType::Colon, ":" );
                    break;
                case '(':
                    addToken( TokenType::LeftParen, "(" );
                    break;
                case ')':
                    addToken( TokenType::RightParen, ")" );
                    break;
                case '{':
                    addToken( TokenType::LeftBrace, "{" );
                    m_isBlockLevel++;
                    break;
                case '}':
                    addToken( TokenType::RightBrace, "}" );
                    m_isBlockLevel--;
                    break;
                case '[':
                    addToken( TokenType::LeftSqBracket, "[" );
                    m_isBlockLevel++;
                    break;
                case ']':
                    addToken( TokenType::RightSqBracket, "]" );
                    m_isBlockLevel--;
                    break;
                case ',':
                    addToken( TokenType::Comma, "," );
                    break;
                case '.':
                    addToken( TokenType::Dot, "." );
                    break;
                case '-':
                    if ( ifNext('=') )
                    {
                        addToken( TokenType::MinusEqual, "-=" );
                    }
                    else if ( ifNext('-') )
                    {
                        auto size = m_tokens.size();
                        if ( size > 0 && m_tokens[size-1].type == TokenType::Identifier )
                        {
                            addToken( TokenType::MinusMinusRight, "--r" );
                        }
                        else
                        {
                            addToken( TokenType::MinusMinusLeft, "--l" );
                        }
                    }
                    else
                    {
                        addToken( TokenType::Minus, "-" );
                    }
                    break;
                case '+':
                    if ( ifNext('=') )
                    {
                        addToken( TokenType::PlusEqual, "=" );
                    }
                    else if ( ifNext('+') )
                    {
                        auto size = m_tokens.size();
                        if ( size > 0 && m_tokens[size-1].type == TokenType::Identifier )
                        {
                            addToken( TokenType::PlusPlusRight, "++r" );
                        }
                        else
                        {
                            addToken( TokenType::PlusPlusLeft, "++l" );
                        }
                    }
                    else
                    {
                        addToken( TokenType::Plus, "+" );
                    }
                    break;
                case ';':
                    addToken( TokenType::Semicolon, ";" );
                    break;
                case '*':
                    if ( ifNext('*') )
                    {
                        addToken(  TokenType::DoubleStar, "**" );
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::StarEqual, "*=" );
                    }
                    else
                    {
                        addToken( TokenType::Star, "*" );
                    }
                    break;
                case '/':
                    if ( ifNext('/') )
                    {
                        addToken(  TokenType::Slash2, "//" );
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::SlashEqual, "/=" );
                    }
                    else
                    {
                        addToken( TokenType::Slash, "/" );
                    }
                    break;
                case '%':
                    if ( ifNext('=') )
                    {
                        addToken(  TokenType::ModEqual, "%=" );
                    }
                    else
                    {
                        addToken( TokenType::Mod, "%" );
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
                        addToken(  TokenType::XorEqual, "^=" );
                    }
                    else
                    {
                        addToken( TokenType::Xor, "^" );
                    }
                    break;
                case '~':
                    addToken( TokenType::Tilde, "~" );
                    break;
                case '!':
                    if ( ifNext('=') )
                    {
                        addToken(  TokenType::NotEqual, "!=" );
                    }
                    else
                    {
                        addToken( TokenType::Not, "!" );
                    }
                    break;
                case '=':
                    if ( ifNext('=') )
                    {
                        addToken(  TokenType::EqualEqual, "==" );
                    }
                    else
                    {
                        addToken( TokenType::Assignment, "=" );
                    }
                    break;
                case '<':
                    if ( ifNext('<') )
                    {
                        if ( ifNext('=') )
                        {
                            addToken( TokenType::LeftShiftEqual, "<<=" );
                        }
                        else
                        {
                            addToken(  TokenType::LeftShift, "<<" );
                        }
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::LessEqual, "<=" );
                    }
                    else
                    {
                        addToken( TokenType::Less, "<" );
                    }
                    break;
                case '>':
                    if ( ifNext('>') )
                    {
                        if ( ifNext('=') )
                        {
                            addToken( TokenType::RightShiftEqual, ">>=" );
                        }
                        else
                        {
                            addToken(  TokenType::RightShift, ">>" );
                        }
                    }
                    else if ( ifNext('=') )
                    {
                        addToken( TokenType::GreaterEqual, ">=" );
                    }
                    else
                    {
                        addToken( TokenType::Greater, ">" );
                    }
                    break;
                 case '#':
                    for( m_ptr++; (m_ptr < m_endPtr) && (*m_ptr != '\n'); m_ptr++ )
                    {
                    }
                    m_ptr--;
                    m_line++;
                    m_pos=-1;
                    break;
                case ' ':
                case '\t':
                    // ignore whitespaces
                    break;
                case '\n':
                case '\r':
                    m_line++;
                    m_pos = -1;
                    addToken( TokenType::Newline, "\n" );
                    handleIndentation( );
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
