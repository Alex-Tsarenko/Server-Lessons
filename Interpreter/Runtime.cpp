#include "Runtime.h"
#include "Expr.h"

void Runtime::initGlobalVariablesR( Namespace& namespaceRef )
{
    m_currentNamespace = &namespaceRef;

    // go through all variables of this namespace
    for( auto& [variableName,var]: namespaceRef.m_variableMap )
    {
        namespaceRef.m_initializationVariableMap[variableName] = true;
        {
            auto value = var->execute(*this,true);
            namespaceRef.m_variableValueMap[ var->m_identifierName ] = value;
        }
        namespaceRef.m_initializationVariableMap[variableName] = false;
    }

    // init nested namespaces
    for( auto& [name,namespaceRef]: namespaceRef.m_innerNamespaceMap )
    {
        initGlobalVariablesR( namespaceRef );
    }
}

void prepareVarMapToInitializationR( Namespace& namespaceRef )
{
    for( auto& [variableName,varDecl]: namespaceRef.m_variableMap )
    {
        namespaceRef.m_initializationVariableMap[variableName] = false;
    }

    for( auto& [namespaceName,namespaceRef]:  namespaceRef.m_innerNamespaceMap )
    {
        prepareVarMapToInitializationR( namespaceRef );
    }
}

void moveVariableValuesToRuntimeR( Namespace& namespaceRef, GlobalVariableMap& globalVariableMap )
{
    globalVariableMap.m_namespaceGlobalVariableMap.clear();
    globalVariableMap.m_namespaceGlobalVariableMap = std::move( namespaceRef.m_variableValueMap );

    for( auto& [namespaceName,namespaceRef]:  namespaceRef.m_innerNamespaceMap )
    {
        globalVariableMap.m_nestedNamespaceMap[namespaceName] = {};
        moveVariableValuesToRuntimeR( namespaceRef, globalVariableMap.m_nestedNamespaceMap[namespaceName] );
    }
}

void Runtime::initGlobalVariables()
{
    initGlobalVariablesR( m_topLevelNamespace );
 
    moveVariableValuesToRuntimeR( m_topLevelNamespace, m_globalVariableMap );
}

void Runtime::run( const std::vector<expr::Expression*>& code, const std::string& sourceCode )
{
    for( auto expr: code )
    {
        expr->execute( *this, false );
    }
}
