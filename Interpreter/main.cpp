#include <iostream>
#include "Lexer.h"
#include "Parser.h"
#include "Executor.h"







const char * test = R"V0G0N(

func test()
{
   var x2 = (1+2)*(n<<2)+--y+0;
   y = x+1;
}

func factorial( n: Int, m: Int )
{
    if ( (n < 0) ) {
        return -1
    }
#    if ( var x ) {
#        raise ArithmeticError()
#    }
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
    
    for( auto& token: lexer.tokens() )
    {
        LOG( gTokenTypeStrings[token.type] )
//        LOG( "#" << gTokenTypeStrings[token.type] << "# " << token.line << " " << token.pos << " "<< token.endPos )
    }
    
    Parser parser;
    parser.parseProgram( test, lexer.tokens() );
    
    return 0;
}
