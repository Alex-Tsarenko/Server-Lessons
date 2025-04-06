#include "ObjectValue.h"
#include "ClassObject.h"

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
