#include <iostream>
#include "Lexer.h"
#include "Parser.h"
#include "Executor.h"







const char * test = R"V0G0N(

func factorial( n: Int )
{
    if ( n < 0 ) {
        raise ArithmeticError()
    }
    if ( var x ) {
        raise ArithmeticError()
    }
    var f = 1
    for ( i=2; i<=n; i++ ) {
        f *= i
    }
    return f
}

print( factorial(8), var i )


)V0G0N";


int main()
{

    Lexer lexer( test );
    lexer.run();
    
    Parser parser;
    parser.parse( lexer.tokens() );
    
    return 0;
}
