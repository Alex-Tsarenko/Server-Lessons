#include <iostream>
#include "Lexer.h"
#include "Parser.h"
#include "Executor.h"
#include "Runtime.h"







const char * test0 = R"V0G0N(

//func f2( a1:Int, a2:Int )
//{
//    var z=1;
//print( "###f2()->\(a1-a2+z)" );
//    return a1-a2+z;
//}

func f3( a1:Int, a2:Int, a3:Int )
{
print( "###f3()->\(a1+a2+a3)" );
    return a1+a2+a3;
}

func f1( a1:Int )
{
    return a1+1;
}

func f0()
{
    return 100;
}

var x1 = f3( 1,f0(),f0());
//var xxx1 = x1;

xxx1 = x1;

//var x0 = f0();
//var x1 = f3( (0), 1+f1( f0()+(2) ), f2(f1(3),f1(4))+5);

print( "x1=\(x1);" );


)V0G0N";


const char * test = R"V0G0N(

class BaseA
{
    x:Int=10;

    constructor( xValue: Int ) {
        x=xValue;
    }
}

class ClassA //: BaseA, private BaseAA
{
    private class InnerClass
    {
        x:Int=0;
    }

    constructor( xValue: Int ) : BaseA(0) {}

    m_int: Int;
    private m_str: String;

    func fPrint()
    {
        print( "ClassA: \(m_int) \(m_str) \n" );
    }
}

func f0()
{
    return 0;
}

var a0=f0();

func f1( a:Int )
{
print( "###f1()->\(a0*a)\n" );
    return a0+a;
}

var dbg=f1(10);

)V0G0N";


int main()
{

    std::string testStr{test};
    Lexer lexer{ testStr.data(), testStr.data()+testStr.size() };
    lexer.run();
    
    for( auto& token: lexer.tokens() )
    {
        LOG( gTokenTypeStrings[token.type] << " " << token.lexeme )
//        LOG( "#" << gTokenTypeStrings[token.type] << "# " << token.line << " " << token.pos << " "<< token.endPos )
    }
    
    Runtime runtime;
    Parser  parser{runtime};
    parser.parseProgram( testStr.data(), lexer.tokens() );
    
    std::vector<expr::Expression*> program = std::move(parser.m_program);
    runtime.run( program, testStr );

    return 0;
}
