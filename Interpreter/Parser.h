#pragma once

#include "Expr.h"
#include <stack>

struct syntax_error: public std::runtime_error
{
    std::string m_error;
    int         m_line;
    int         m_position;
    int         m_endPosition;

    syntax_error( const std::string& error, int line, int position, int end ) : std::runtime_error(error),
      m_error(error),
      m_line(line),
      m_position(position),
      m_endPosition(end)
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
    
    // a+(0+(b+c))
    // [+ a (+ 0 (+ b) c]
    struct BinaryPraser
    {
        
    };
    std::stack<expr::ExpressionPtr> m_exprStack;
    
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
            for( int i=0; i<error.m_position-1; i++ )
            {
                std::cerr << ' ';
            }
            for( int i=error.m_position; i<error.m_endPosition; i++ )
            {
                std::cerr << '^';
            }
            std::cerr << std::endl;
        }
    }

protected:

    void parseStatement()
    {
        skipNewLines();
        
        if ( isEof() )
        {
            //TODO ')' ',' ';'
            if ( m_blockLevel != 0 )
            {
                throw syntax_error( "unexpected end of file, expected '}'", (m_tokenIt-1)->line, (m_tokenIt-1)->pos, (m_tokenIt-1)->endPos );
            }
            return;
        }

        switch ( m_tokenIt->type ) {
            case Var:
                parseVar();
                break;
            case Func:
                parseFuncDef();
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
                    throw syntax_error( std::string("unexpected ")+gTokenTypeStrings[RightBrace], (m_tokenIt-1)->line, (m_tokenIt-1)->pos, (m_tokenIt-1)->endPos );
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
            if ( isEof() )
            {
                throw syntax_error( "unexpected end of file", (m_tokenIt-1)->line, (m_tokenIt-1)->pos, (m_tokenIt-1)->endPos );
            }
        }
        // Убрать Newline-ы чтобы не мешали дальнейшему парсингу
        while( m_tokenIt->type == Newline );
    }
    
    void tokenMustBe( TokenType type )
    {
        if ( m_tokenIt->type != type )
        {
            throw syntax_error( std::string("expected: '")+gTokenTypeStrings[type]+"'", m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
    }
    
    void nextToken( TokenType type )
    {
        nextToken();
        if ( m_tokenIt->type != type )
        {
            throw syntax_error( std::string("expected: '")+gTokenTypeStrings[type]+"'", m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
    }
    
    bool isEof()
    {
        return m_tokenIt == m_tokenEnd;
    }

    void skipNewLines()
    {
        while( !isEof() && m_tokenIt->type == Newline )
        {
            m_tokenIt++;
        }
    }

    bool tokenIs( TokenType type )
    {
        return m_tokenIt->type == type;
    }
    
    bool nextTokenIs( TokenType type )
    {
        return ((m_tokenIt+1) != m_tokenEnd)  &&  ((m_tokenIt+1)->type == type);
    }
    
    void tokenMustBeType()
    {
        switch( m_tokenIt->type )
        {
            case Int:
            case Float:
            case String:
                break;
                
            default:
                throw syntax_error( std::string("expected type: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
        }
    }

    
    expr::ExpressionPtr parseExpr()
    {
        static char opratorTokenTable[EndOfTokenType] = {0};
        if ( opratorTokenTable[Plus] == 0 )
        {
            opratorTokenTable[PlusPlusRight] = 2;
            opratorTokenTable[MinusMinusRight] = 2;
            opratorTokenTable[PlusPlusLeft] = 3;
            opratorTokenTable[MinusMinusLeft] = 3;
            opratorTokenTable[Not] = 3;
            opratorTokenTable[Tilde] = 3;
            opratorTokenTable[Ampersand] = 3;
            opratorTokenTable[Star] = 5;
            opratorTokenTable[Slash] = 5;
            opratorTokenTable[Mod] = 5;
            opratorTokenTable[Plus] = 6;
            opratorTokenTable[Minus] = 6;
            opratorTokenTable[Less] = 9;
            opratorTokenTable[LessEqual] = 9;
            opratorTokenTable[Greater] = 9;
            opratorTokenTable[GreaterEqual] = 9;
            opratorTokenTable[EqualEqual] = 10;
            opratorTokenTable[NotEqual] = 10;
            opratorTokenTable[Ampersand2] = 14;
            opratorTokenTable[Or2] = 15;
        }

        std::stack<expr::ExpressionPtr> output;
        std::stack<expr::ExpressionPtr> opStack;
        for(;;)
        {
            nextToken();

            switch( m_tokenIt->type )
            {
                case Identifier:
                {
                    break;
                }
                case Number:
                {
                    break;
                }
                case LeftParen:
                {
                    break;
                }
                case RightParen:
                {
                    break;
                }
                default:
                {
                    auto precedence = opratorTokenTable[m_tokenIt->type];
                    if ( precedence == 0 )
                    {
                        throw syntax_error( std::string("unexpected token: ")+gTokenTypeStrings[m_tokenIt->type], m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                    }
                }
            }
        }
    }
    
    void parseFuncCall()
    {
        //todo
    }

    void parseVar()
    {
        //
        // "var" <indentifier> [ "=" <expression> ]
        //
        nextToken( Identifier );
        auto& varName = m_tokenIt->lexeme;
        expr::ExpressionPtr expr = nullptr;
        if ( nextTokenIs( Assignment ) )
        {
            // skip assignment
            nextToken();
            expr = parseExpr();
        }
        
        auto* varExpr = new expr::ExpressionVarDecl{varName};
        varExpr->m_expr = std::move(expr);
    }

    void parseFuncDef()
    {
//        int y=2;
//        
//        int n=5;
//        int x = 1*n^2;//+--y;
//        LOG("x: " << x )

        nextToken();
        expr::Func* func = new expr::Func;
        m_current->m_list.push_back( func );
        
        // Save function name
        func->m_name = m_tokenIt->lexeme;
        
        nextToken( LeftParen );
        nextToken();
        
        // Parse arguments
        int argumentNumber = 0;
        while( m_tokenIt->type != RightParen )
        {
            if ( m_tokenIt->type != Identifier )
            {
                if ( argumentNumber == 0 )
                {
                    throw syntax_error( std::string("expected ')': "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                }
                throw syntax_error( std::string("expected argument name: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
            }

            // Argument name
            std::string name = m_tokenIt->lexeme;
            
            nextToken( Colon );

            // Parse argument type
            nextToken();
            tokenMustBeType();
            
            LOG( "argument: " << name << " Type:" << m_tokenIt->lexeme )
            func->m_argList.emplace_back( expr::Argument{std::move(name), m_tokenIt->lexeme} );

            nextToken();
            if ( ! tokenIs( Comma ) )
            {
                tokenMustBe( RightParen );
                break;
            }
            nextToken();

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
        nextToken( LeftParen );
        
        throw syntax_error( std::string("TODO: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );

    }


};
