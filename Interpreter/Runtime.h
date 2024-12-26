#pragma once

#include "TokenType.h"
#include "TokenTypeStrings.h"
#include <map>
#include <stack>

namespace expr
{
    struct FuncDefinition;
    struct ExpressionVarDecl;
    struct Expression;
}

f2(x)
{
    
}
f1( x)
{
    f2(10)
}

struct Runtime
{
    Runtime() {}
    
    std::map<std::string,expr::FuncDefinition*>     m_funcMap;
    std::map<std::string,Object>                    m_varMap;
    
    std::stack< std::map<std::string,Object> >      m_localVarStack;
    
    void run( const std::vector<expr::Expression*>& code );
};
