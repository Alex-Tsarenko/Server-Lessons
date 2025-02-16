#pragma once

#include "TokenType.h"
#include "TokenTypeStrings.h"
#include "ClassObject.h"
#include "Namespace.h"
#include <map>
#include <stack>

namespace expr
{
    struct FuncDefinition;
    struct VarDeclaration;
    struct ClassDefinition;
    struct Expression;
}

struct ObjectValue;

//struct Namespace
//{
//    std::map<std::string,Namespace> m_nestedNamespaces;
//    std::map<std::string,expr::VarDeclaration*> m_vars;
//    std::map<std::string,expr::FuncDefinition*> m_funcs;
//    std::map<std::string,expr::ClassDefinition*> m_classes;
//};

struct Runtime
{
    Runtime() {}
    
    Namespace m_namespace;

    std::map<std::string,ObjectValue> m_globalVariableMap;

    using LocalVarStack = std::vector< std::map<std::string,ObjectValue> >;
    LocalVarStack                     m_localVarStack;

    void run( const std::vector<expr::Expression*>& code, const std::string& sourceCode );
};
