#pragma once

#include <map>
#include <string>
#include "ObjectValue.h"

struct ClassObject
{
    ~ClassObject() { m_members.clear(); }

    struct ClassMember {
        enum Type { value, shared_ptr, weak_ptr };
        Type m_type;

        ~ClassMember()
        {
            switch( m_type )
            {
                case value:
                    m_value.~ObjectValue();
                    break;
                case shared_ptr:
                    using namespace std;
                    m_sharedPtrValue.~shared_ptr<ObjectValue>();
                    break;
                case weak_ptr:
                    using namespace std;
                    m_weakPtrValue.~weak_ptr<ObjectValue>();
                    break;
            }
        }

        union {
            ObjectValue                  m_value;
            std::shared_ptr<ObjectValue> m_sharedPtrValue;
            std::weak_ptr<ObjectValue>   m_weakPtrValue;
        };
    };
    expr::ClassDefinition*              m_definition; // for accessto funcs
    std::map<std::string,ClassMember>   m_members;
};

//class X
//{
//    ClassA      m_a;
//    ClassA&     m_aPtr;
//    ClassA&&    m_aPtr;
//}
