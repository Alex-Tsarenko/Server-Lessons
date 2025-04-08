
func main()
{
    //::A1::A2::test();
    //print("TEST: \(A1::A2::f11())\n";

    var a = ClassA;
    var a2 = ::ClassA();
    //var a: Int = 100;
    print("TEST: \(a.m_int)\n";
    //print("TEST: \(a)\n";
}

class ClassA
{
    func ClassA() {}

    var m_int: 0;
    
    func f() { return "class-a";}
}

namespace A1
{
//    func f10001( abc ) { return abc; }
    func f11() { return 101; }
    var x = "var-x";


    namespace A2
    {
        //func test() { A1::f11(); }
        func test() { print("TEST: \(::A1::f11()) \(::A1::x)             \n"; }

        namespace A1
        {
            func f11() { return 111; }
        }
    }
}

//print("TEST: \(::A1::f11())\n";

//namespace A1
//{
//    func f11() { return 111; }
//
//    namespace A2
//    {
//        func f11() { return 112; }
//
//        namespace A3
//        {
//            func f11() { return 113; }
//            namespace A2
//            {
//                func f11() { return 1122; }
//            }
//
//            func f()
//            {
//                //print( "\(::A1::A2::f11())" );
//            }
//        }
//    }
//}

//class BaseA
//{
//    x:Int=10;
//
//    constructor( xValue: Int ) {
//        x=xValue;
//    }
//}
//
//class ClassA : BaseA, private BaseAA
//{
//    private class InnerClass
//    {
//        x:Int=0;
//    }
//
//    constructor( xValue: Int ) //: BaseA(0)
//    {
//        var m_int = 10;
//        m_str = "str";
//    }
//
//    m_int: Int;
//    private m_str: String;
//
//    func fPrint()
//    {
//        //print( "ClassA: \(m_int) \(m_str) \n" );
//        var x=0;
////        if (x)
////        {
////            var x=1;
////        }
//    }
//}
//
//var dbg200=f100();
//
//func main()
//{
//    //print( "XXX: \(dbg200) \(dbg200) \n" );
//}
//
//func f100()
//{
//    return 100;
//}

//func main()
//{
//    var x = 10 + f(4,a*(8+3),x-f12()) + 11;
//    print("\(f11())";
////    A1::A2::A3::f(0);
//}

