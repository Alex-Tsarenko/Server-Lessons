#pragma once

#include "Expr.h"
#include <stack>
#include <cassert>

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
    
    std::vector<expr::Expression*> m_exprOutput;
    std::vector<expr::Expression*> m_operationStack;
    bool unariOpratorTokenTable[EndOfTokenType] = {false};

    
public:
    Parser() {}
    
    void onError( std::string errorText, int line, int pos ) {}

    void parseProgram( const char* programText, const std::vector<Token>& tokens )
    {
        m_tokenIt  = tokens.begin();
        m_tokenEnd = tokens.end();

        m_current = &m_program;
        
        assert( tokens.size() > 0 );
        assert( tokenIs(Begin) );
        nextToken();

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
        return m_tokenIt->type == EndOfFile;
        //return m_tokenIt == m_tokenEnd;
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

    
    expr::Expression* parseExpr()
    {
        // https://en.cppreference.com/w/cpp/language/operator_precedence
        
        if ( ! unariOpratorTokenTable[Not] )
        {
            unariOpratorTokenTable[Not] = true;
            unariOpratorTokenTable[PlusPlusRight] = true;
            unariOpratorTokenTable[PlusPlusLeft] = true;
            unariOpratorTokenTable[MinusMinusRight] = true;
            unariOpratorTokenTable[MinusMinusLeft] = true;
        }

        static char opratorTokenTable[EndOfTokenType] = {0};
        if ( opratorTokenTable[Plus] == 0 )
        {
            opratorTokenTable[LeftParen] = 100;

            opratorTokenTable[PlusPlusRight] = 2;
            opratorTokenTable[MinusMinusRight] = 2;
            opratorTokenTable[PlusPlusLeft] = 3;
            opratorTokenTable[MinusMinusLeft] = 3;
            opratorTokenTable[Not] = 3;
            opratorTokenTable[Tilde] = 3;
            //opratorTokenTable[Ampersand] = 3;
            opratorTokenTable[Star] = 5;
            opratorTokenTable[And] = 5;
            opratorTokenTable[Mod] = 5;
            opratorTokenTable[Plus] = 6;
            opratorTokenTable[Minus] = 6;
            opratorTokenTable[LeftShift] = 6;
            opratorTokenTable[RightShift] = 6;
            opratorTokenTable[Less] = 9;
            opratorTokenTable[LessEqual] = 9;
            opratorTokenTable[Greater] = 9;
            opratorTokenTable[GreaterEqual] = 9;
            opratorTokenTable[EqualEqual] = 10;
            opratorTokenTable[NotEqual] = 10;
            opratorTokenTable[BitAnd] = 11;
            opratorTokenTable[Xor] = 11;
            opratorTokenTable[BitOr] = 13;
            opratorTokenTable[And] = 14;
            opratorTokenTable[Or] = 15;
        }

        assert( m_exprOutput.size() == 0 );
        assert( m_operationStack.size() == 0 );

        static expr::Expression leftParen{Token{LeftParen,"("}};
        static expr::Expression rightParen{Token{RightParen,")"}};;

        bool theEnd = false;
        
        while( !theEnd )
        {
            nextToken();

            LOG( "parseExpr: " << gTokenTypeStrings[m_tokenIt->type] );
            switch( m_tokenIt->type )
            {
                case Identifier:
                {
                    m_exprOutput.push_back( new expr::Identifier{ *m_tokenIt } );
                    break;
                }
                case Int:
                {
                    m_exprOutput.push_back( new expr::IntNumber{ *m_tokenIt } );
                    break;
                }
                case Float:
                {
                    m_exprOutput.push_back( new expr::FloatNumber{ *m_tokenIt } );
                    break;
                }
                case LeftParen:
                {
                    if ( (m_tokenIt-1)->type == Identifier )
                    {
                        
                    }
                    // Before '(' must be <function name> or '=' or <sign/operation>
                    else if ( opratorTokenTable[(m_tokenIt-1)->type] == 0
                             && (m_tokenIt-1)->type != LeftParen
                             && (m_tokenIt-1)->type != Comma
                             && (m_tokenIt-1)->type != Assignment )
                    {
                        throw syntax_error( std::string("unexpected left parenthesis: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                    }
                    m_operationStack.push_back( &leftParen );
                    break;
                }
                case RightParen:
                {
                    for(;;)
                    {
                        if ( m_operationStack.size() < 0 )
                        {
                            throw syntax_error( std::string("unexpected right parenthesis: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                        }
                            
                        if ( m_operationStack.back() == &leftParen )
                        {
                            //output.push_back( std::move( opStack.back() ) );
                            m_operationStack.pop_back();
                            break;
                        }
                        
                        m_exprOutput.push_back( std::move( m_operationStack.back() ) );
                        m_operationStack.pop_back();
                    }
                    break;
                }
                case Comma:
                {
                    //TODO:
                    assert(0);
                    break;
                }
                case LeftSqBracket:
                {
                    //TODO:
                    assert(0);
                    break;
                }
                case RightSqBracket:
                {
                    //TODO:
                    assert(0);
                    break;
                }
                case Semicolon:
                {
                    while( m_operationStack.size() > 0 )
                    {
                        if ( m_operationStack.back() == &leftParen )
                        {
                            throw syntax_error( std::string("missing ')': "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                        }
                        m_exprOutput.push_back( std::move( m_operationStack.back() ) );
                        m_operationStack.pop_back();
                    }
                    
                    if ( m_exprOutput.size() == 0 )
                    {
                        throw syntax_error( std::string("expected expression: "), m_tokenIt->line, m_tokenIt->pos, m_tokenIt->endPos );
                    }

                    auto result = constructExpr();
                    
                    theEnd = true;
                    break;
                }
                default:
                {
                    auto precedence = opratorTokenTable[m_tokenIt->type];
                    if ( precedence == 0 )
                    {
                        theEnd = true;
                        break;
                    }
                    
                    while( m_operationStack.size() > 0 )
                    {
                        if ( opratorTokenTable[(m_operationStack.back())->m_token.type] <= precedence )
                        {
                            m_exprOutput.push_back( std::move( m_operationStack.back() ) );
                            m_operationStack.pop_back();
                        }
                        else
                        {
                            break;
                        }
                    }
                    if ( unariOpratorTokenTable[m_tokenIt->type] )
                    {
                        m_operationStack.push_back( new expr::UnaryOpExpression(*m_tokenIt) );
                    }
                    else
                    {
                        m_operationStack.push_back( new expr::BinaryOpExpression(*m_tokenIt) );
                    }
                }
            }
        }
    }
    
    expr::Expression* constructExpr()
    {
        if ( m_exprOutput.empty() )
        {
            return nullptr;
        }
        
        auto* expr = m_exprOutput.back();
        m_exprOutput.pop_back();
        
        LOG( "constructExpr: " << gTokenTypeStrings[expr->m_token.type] << " :" << m_exprOutput.size() )
        
        if ( m_exprOutput.size() == 0 )
        {
            LOG("");
        }
        
        if ( unariOpratorTokenTable[expr->m_token.type] )
        {
            ((expr::UnaryExpression*)expr)->m_expr = constructExpr();
            if ( ((expr::UnaryExpression*)expr)->m_expr == nullptr )
            {
                auto& token = expr->m_token;
                throw syntax_error( std::string("expression syntax error: "), token.line, token.pos, token.endPos );
            }
        }
        else if ( expr->m_token.type == Identifier || expr->m_token.type == Int || expr->m_token.type == Float )
        {
            return expr;
        }
        else
        {
            ((expr::BinaryOpExpression*)expr)->m_expr = constructExpr();
            if ( ((expr::BinaryOpExpression*)expr)->m_expr == nullptr )
            {
                auto& token = expr->m_token;
                throw syntax_error( std::string("expression syntax error: "), token.line, token.pos, token.endPos );
            }
            
            ((expr::BinaryOpExpression*)expr)->m_expr2 = constructExpr();
            if ( ((expr::BinaryOpExpression*)expr)->m_expr2 == nullptr )
            {
                auto& token = expr->m_token;
                throw syntax_error( std::string("expression syntax error: "), token.line, token.pos, token.endPos );
            }
        }
        return expr;
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
        const Token& varName = *m_tokenIt;
        expr::Expression* expr = nullptr;
        if ( nextTokenIs( Assignment ) )
        {
            // skip assignment
            nextToken();
            expr = parseExpr();
        }
        
        auto* varExpr = new expr::ExpressionVarDecl{varName};
        varExpr->m_expr = expr;
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
