#pragma once

#include <map>
#include <string>

struct Namespace
{
    std::map< std::string, expr::VarDeclaration*>   m_variableMap;
    std::map<std::string,expr::FuncDefinition*>     m_functionMap;
    std::map<std::string,expr::ClassDefinition*>    m_classMap;

    std::map< std::string, Namespace>               m_innerNamespaceMap;
};
