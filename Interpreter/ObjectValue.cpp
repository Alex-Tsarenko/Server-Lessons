#include "ObjectValue.h"
#include "ClassObject.h"
#include "Expr.h"

ObjectValue::~ObjectValue()
{
    using namespace std;

    if ( m_type == ot_string )
    {
        delete m_stringValue;
    }
    else if ( m_type == ot_class_ptr )
    {
        delete m_classPtr;
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
    value.m_classPtr = new ClassObject{ &classDef };

    for( auto [name,varDecl] : classDef.m_variableMap )
    {
        ObjectValue varValue = varDecl->execute( runtime, isGlobal );
        LOG( "varValue.m_classPtr-> " << name )
        value.m_classPtr->m_members.emplace( name, varValue );
        LOG( "2 varValue.m_classPtr-> " << name )

    }

    return value;
}
