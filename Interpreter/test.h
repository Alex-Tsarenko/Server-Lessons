
//var a = ns::ClassA;

func main()
{
    //::A1::A2::test();
    //var xxx:int;
    //print("TEST: \(A1::A2::f11())\n";

    var a = ClassA;
    //var a = ::ns::ClassA();
    //print("TEST: \(xxx)\n";

    a = ::ns::ClassA;
    print("TEST: \(a.m_int)\n";
}

class ClassA
{
    func ClassA() {}

    var m_int = "--ффф--";

    func f() { return "class-a";}
}

namespace ns
{
    class ClassA
    {
        func ClassA() {}

        var m_int = "--110--";

        func f() { return "class-a";}
    }
}

