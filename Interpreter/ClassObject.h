#pragma once

#include <map>
#include <string>
#include "ObjectValue.h"
#include "Expr.h"


struct ClassObject
{
    expr::ClassOrNamespace*                 m_definition; // must be class
    std::map<std::string_view,ObjectValue>  m_members;

    ClassObject( expr::ClassOrNamespace* definition ) : m_definition(definition)
    {
//        for( auto& [name,varDecl] : definition->m_variableMap )
//        {
//            m_members[name] = ObjectValue();
//            if ( varDecl->m_initValue != nullptr )
//            {
//                //varDecl->m_initValue->execute( ru, <#bool isGlobal#>)
//            }
//        }
    }

    ~ClassObject() {
        m_members.clear();
    }
};
