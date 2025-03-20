#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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


// interpreter file1 file2 file3

const char * test = R"V0G0N(

var g1=g2+1;
var g2=g1+1;

class BaseA
{
    x:Int=10;

    constructor( xValue: Int ) {
        x=xValue;
    }
}

class ClassA : BaseA, private BaseAA
{
    private class InnerClass
    {
        x:Int=0;
    }

    constructor( xValue: Int ) //: BaseA(0)
    {
        var m_int = 10;
        m_str = "str";
    }

    m_int: Int;
    private m_str: String;

    func fPrint()
    {
        //print( "ClassA: \(m_int) \(m_str) \n" );
        var x=0;
//        if (x)
//        {
//            var x=1;
//        }
    }
}

var dbg200=f100();

func main()
{
    //print( "XXX: \(dbg200) \(dbg200) \n" );
}

func f100()
{
    return 100;
}
)V0G0N";

///////////////////////////////////////////////////////////////////////////////
const char * test_old = R"V0G0N(

//f100()
//{
//    return 100;
//}

//var dbg200=f100()*2;

//main()
//{
//    print( "XXX: \(dbg200) \(dbg200) \n" );
//}


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

    constructor( xValue: Int ) //: BaseA(0)
    {
        var& m_int = 10;
        m_str = "str";
    }

    m_int: Int;
    private m_str: String;

    func fPrint()
    {
        print( "ClassA: \(m_int()) \(m_str) \n" );
        var x=0
//        if (x)
//        {
//            var x=1;
//        }
    }
}

var a2: ClassA;
a2.fPrint();


func f0()
{
    class X {}
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


std::string readFile( std::string fileName )
{
    // Open the file in input mode
        std::ifstream file( fileName );

        // Check if the file was opened successfully
        if ( !file.is_open() )
        {
            throw std::runtime_error( "Could not open file: " + fileName );
        }

        // Use a stringstream to read the entire file content
        std::stringstream buffer;
        buffer << file.rdbuf();  // Read the file's buffer contents into the stringstream

        // Convert the stringstream content to a string
        return buffer.str();
}

//int aa=0;
//
//namespace expr1
//{
//    int b=1;
//    namespace expr1
//    {
//        int c=2;
//        namespace expr2
//        {
//            int d = ::aa + ::expr1::b + expr1::c;
//        }
//
//    }
//
//}





int main()
{
    std::string testStr = readFile("/Users/alex2/C++/Server-Lessons/Interpreter/test.h");
    Lexer lexer{ testStr.data(), testStr.data()+testStr.size() };
    lexer.run();
    
    for( auto& token: lexer.tokens() )
    {
        if (  token.type == IdentifierWithScope )
        {
            LOG( "#### " << gTokenTypeStrings[token.type] << "   " << token.lexeme )
        }
//        LOG( "#### " << gTokenTypeStrings[token.type] << " # " << token.line << " " << token.pos << " "<< token.endPos )
    }

    Namespace globalNamespace;
    Parser  parser{globalNamespace};
    parser.parseProgram( testStr.data(), lexer.tokens() );
    
    try
    {
        Runtime runtime( testStr, globalNamespace );
        runtime.initGlobalVariables();
        std::vector<expr::Expression*> program = std::move(parser.m_program);
        runtime.run( program, testStr );
    }
    catch( const runtime_error& error )
    {
        std::cerr << "!!! Runtime error: " << error.what() << std::endl;
//        std::cerr << "\n!!!! Runtime error: line: " << error.m_line << std::endl;
//        std::cerr << "\n!!!! Runtime error: pos: " << error.m_position << std::endl;
        std::cerr << getLine( testStr.c_str(), error.m_line ) << std::endl;
        for( int i=0; i<error.m_position; i++ )
        {
            std::cerr << ' ';
        }
        for( int i=error.m_position; i<error.m_endPosition; i++ )
        {
            std::cerr << '^';
        }
        std::cerr << std::endl;
    }

    return 0;
}
