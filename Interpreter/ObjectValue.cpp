#include "ObjectValue.h"
#include "ClassObject.h"

ObjectValue::~ObjectValue()
{
    if ( m_type == ot_string )
    {
        delete m_stringValue;
    }
    else if ( m_type == ot_class )
    {
        delete m_classObject;
    }
}
