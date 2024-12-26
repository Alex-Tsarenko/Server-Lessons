#include "Runtime.h"
#include "Expr.h"

void Runtime::run( const std::vector<expr::Expression*>& code )
{
    for( auto expr: code )
    {
        expr->execute( *this );
    }
}
