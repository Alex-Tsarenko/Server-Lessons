#include "ObjectValue.h"
#include "ClassObject.h"
#include "Expr.h"

ObjectValue gNullObject;


ObjectValue::~ObjectValue()
{
    using namespace std;

    if ( m_type == ot_string )
    {
        m_stringValue.~string();
    }
    else if ( m_type == ot_class_ptr )
    {
        delete m_classObjPtr;
    }
    else if ( m_type == ot_class_shared_ptr )
    {
        m_classSharedPtr.~shared_ptr<ClassObject>();
    }
    else if ( m_type == ot_class_weak_ptr )
    {
        m_classWeakPtr.~weak_ptr<ClassObject>();
    }
}

void createClassObject( ObjectValue& outValue, Runtime& runtime, bool isGlobal, expr::ClassOrNamespace& classDef )
{
    assert( outValue.m_type == ot_null );

    outValue.m_type = ot_class_shared_ptr;

//    new (&outValue.m_classSharedPtr) std::shared_ptr<ClassObject>( std::make_shared<ClassObject>( &classDef ));
    new (&outValue.m_classSharedPtr) std::shared_ptr<ClassObject>( new ClassObject(&classDef) );

    for( auto [name,varDecl] : classDef.m_variableDeclMap )
    {
        ObjectValue varValue;
        varDecl->execute( varValue, runtime, isGlobal );
        outValue.m_classSharedPtr->m_members.emplace( name, varValue );
    }
}
