weak_ptr ptr;
var shared;

func f()
{
    var sharedClassA = ClassA;
    ptr = sharedClassA;
//    shared = ptr;
//    print("TEST: \(shared.m_str)\n";
//    print("TEST: \(ptr.m_str)\n";
}

 func main()
 {
    f();
    shared = ptr;
    //weak_ptr ptr2 = ptr;
    //print("TEST: \(shared.m_str)\n";
 }

// var aGlobal = ClassA;


class ClassA
{
   func ClassA() {}

   //func destructor() {  print("destructor"); }

   var m_str = "--ффф--";
}


//namespace ns
//{
//    class ClassA
//    {
//        func ClassA() {}
//
//        var m_int = "--110--";
//
//        func f() { return "class-a";}
//    }
//}



//
//class OwnerClass
//{
//    var m_member = OwnerClass2(self);
//}
//
//class MemeberClass
//{
//    weak_ref m_backRef: OwnerClass;
//
//    MemeberClass( backRef: OwnerClass ) { m_backRef = backRef; }
//}
//
//func main()
//{
//    if ( (it,it2) = aHead.find(101) )
//    {
//    }
//
//    if ( aHead.m_next )
//    {
//        print("TEST: \(aHead.m_next.m_int)\n";
//        //...
//    }
//
//    try {
//        print("TEST: \(aHead.m_next.m_int)\n";
//        //...
//    }
//
//    print("TEST: \(aHead.m_next.m_int)\n" ?? ;
//
//    ii = aHead.m_next.m_int ?? 0;
//
////    a = ::ns::ClassA;
////    print("TEST: \(a.m_int)\n";
//}

//class ClassA
//{
//    func ClassA() {}
//
//    //func destructor() {  print("destructor"); }
//
//    var m_int = "--ффф--";
//
//    func f() { return "class-a";}
//}
//
//namespace ns
//{
//    class ClassA
//    {
//        func ClassA() {}
//
//        var m_int = "--110--";
//
//        func f() { return "class-a";}
//    }
//}



//var shared = ClassA;
//weak_ptr shared2 = shared;
//
//func f5()
//{
//    if ( not a )
//    {
//        return_error ("not a",-1);
//    }
//
//    var aCopy = a.copy();
//
//    weak_ptr aRef = a;
//
//    return 101;
//}
//
//func f6()
//{
//    if ( err = fun().error_code() )
//    {
//
//    }
//}
