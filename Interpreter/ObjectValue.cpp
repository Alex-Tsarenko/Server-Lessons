#include "ObjectValue.h"
#include "ClassObject.h"
#include "Expr.h"

ObjectValue gNullObject;


ObjectValue::~ObjectValue()
{
    using namespace std;

    if ( m_type == ot_string )
    {
        delete m_stringValue;
    }
    else if ( m_type == ot_class_ptr )
    {
        delete m_classObjPtr;
    }
    else if ( m_type == ot_class_shared_ptr )
    {
        delete m_classSharedPtr;
    }
    else if ( m_type == ot_class_weak_ptr )
    {
        delete m_clasWeakPtr;
    }
}

ObjectValue createClassObject( Runtime& runtime, bool isGlobal, expr::ClassOrNamespace& classDef )
{
    ObjectValue value;
    value.m_type = ot_class_ptr;
    value.m_classObjPtr = new ClassObject{ &classDef };

    for( auto [name,varDecl] : classDef.m_variableMap )
    {
        ObjectValue varValue;
        varDecl->execute( varValue, runtime, isGlobal );
        value.m_classObjPtr->m_members.emplace( name, varValue );
    }

    return value;
}
