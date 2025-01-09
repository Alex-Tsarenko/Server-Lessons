#include "Runtime.h"
#include "Expr.h"

void Runtime::run( const std::vector<expr::Expression*>& code, const std::string& sourceCode )
{
    try
    {
        for( auto expr: code )
        {
            expr->execute( *this );
        }
    }
    catch( runtime_ex3& error )
    {
        //sourceCode
        std::cerr << "\n!!! Runtime error: " << error.what() << std::endl;
        std::cerr << getLine( sourceCode.c_str(), error.m_token.line ) << std::endl;
        for( int i=0; i<error.m_token.pos; i++ )
        {
            std::cerr << ' ';
        }
        for( int i=error.m_token.pos; i<error.m_token.endPos; i++ )
        {
            std::cerr << '^';
        }
        std::cerr << std::endl;
    }
}
