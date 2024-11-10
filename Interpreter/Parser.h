#pragma once

#include "Expr.h"

struct syntax_error: public std::runtime_error
{
    std::string m_error;
    int         m_line;
    int         m_position;
    
    syntax_error( const std::string& error, int line, int position ) : std::runtime_error(error),
      m_error(error),
      m_line(line),
      m_position(position)
    {}

//    runtime_error(const runtime_error&);
//    runtime_error& operator=(const runtime_error&);

    ~syntax_error() override {}
};

class Parser
{
    expr::ExpressionList    m_program;
    expr::ExpressionList*   m_current;

    std::vector<Token>::const_iterator m_tokenIt;
    std::vector<Token>::const_iterator m_tokenEnd;
    
    int m_blockLevel = 0;
    
public:
    Parser() {}
    
    void onError( std::string errorText, int line, int pos ) {}

    void parseProgram( const char* programText, const std::vector<Token>& tokens )
    {
        m_tokenIt  = tokens.begin();
        m_tokenEnd = tokens.end();

        m_current = &m_program;
        
        try
        {
            for( const auto& it : tokens )
            {
                parseStatement();
            }
        }
        catch( syntax_error& error )
        {
            std::cerr << "!!! Syntax error: " << error.what() << std::endl;
            std::cerr << getLine( programText, error.m_line+1 ) << std::endl;
            for( int i=0; i<error.m_position; i++ )
            {
                std::cerr << ' ';
            }
            std::cerr << '^'  << std::endl;
        }
    }

protected:
    void parseStatement()
    {
        while( m_tokenIt != m_tokenEnd && m_tokenIt->type == Newline )
        {
            m_tokenIt++;
        }
        
        if ( m_tokenIt == m_tokenEnd )
        {
            //TODO ')' ',' ';'
            if ( m_blockLevel != 0 )
            {
                throw syntax_error( "unexpected end of file, expected '}'", (m_tokenIt-1)->line, -1 );
            }
            return;
        }

        switch ( m_tokenIt->type ) {
            case Func:
                parseFunc();
                break;
            case If:
                parseIf();
                break;
            case For:
                break;
            case Return:
                break;
            case Print:
                break;

            case RightBrace:
                if ( m_blockLevel == 0 )
                {
                    throw syntax_error( std::string("unexpected ")+gTokenTypeStrings[RightBrace], (m_tokenIt-1)->line, -1 );
                }
                m_tokenIt--;
                break;

            default:
                break;
        }
    }

    void nextToken()
    {
        do
        {
            m_tokenIt++;
            if ( m_tokenIt == m_tokenEnd )
            {
                throw syntax_error( "unexpected end of file", (m_tokenIt-1)->line, -1 );
            }
        }
        while( m_tokenIt->type == Newline );
    }
    
    void tokenMustBe( TokenType type )
    {
        if ( m_tokenIt->type != type )
        {
            throw syntax_error( std::string("expected: ")+gTokenTypeStrings[type], m_tokenIt->line, m_tokenIt->pos );
        }
    }
    
    void nextToken( TokenType type )
    {
        nextToken();
        if ( m_tokenIt->type != type )
        {
            throw syntax_error( std::string("expected: ")+gTokenTypeStrings[type], m_tokenIt->line, m_tokenIt->pos );
        }
    }
    
    bool tokenIs( TokenType type )
    {
        return m_tokenIt->type == type;
    }
    
    void tokenMustBeType()
    {
        nextToken();
        switch( m_tokenIt->type )
        {
            case Int:
            case Float:
            case String:
                break;
                
            default:
                throw syntax_error( std::string("expected type: "), m_tokenIt->line, m_tokenIt->pos );
        }
    }
    
    void nextTokenMustBeType()
    {
        nextToken();
        switch( m_tokenIt->type )
        {
            case Int:
            case Float:
            case String:
                break;
                
            default:
                throw syntax_error( std::string("expected type: "), m_tokenIt->line, m_tokenIt->pos );
        }
    }
    
    void parseFunc()
    {

        nextToken();
        expr::Func* func = new expr::Func;
        m_current->m_list.push_back( func );
        
        // Save function name
        func->m_name = m_tokenIt->lexeme;
        
        nextToken( LeftParen );
        nextToken();
        
        int argumentNumber = 0;
        while( m_tokenIt->type != RightParen )
        {
            if ( m_tokenIt->type != Identifier )
            {
                if ( argumentNumber == 0 )
                {
                    throw syntax_error( std::string("expected ')': "), m_tokenIt->line, m_tokenIt->pos );
                }
                throw syntax_error( std::string("expected argument name: "), m_tokenIt->line, m_tokenIt->pos );
            }

            std::string name = m_tokenIt->lexeme;
            
            nextToken( Colon );

            nextTokenMustBeType();
            
            func->m_argList.emplace_back( expr::Argument{std::move(name), m_tokenIt->lexeme} );
            nextToken();

            if ( ! tokenIs( Comma ) )
            {
                tokenMustBe( RightParen );
                break;
            }
        }

        nextToken( LeftBrace );
        m_blockLevel++;
        
        auto oldContainer = m_current;
        m_current = &func->m_body;

        nextToken();
        while( m_tokenIt->type != RightBrace )
        {
            parseStatement();
        }

        m_current = oldContainer;
        m_blockLevel--;
        
        m_tokenIt++;
    }
    
    void parseIf()
    {
        throw syntax_error( std::string("TODO: "), m_tokenIt->line, m_tokenIt->pos );

    }


};
