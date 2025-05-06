#pragma once

#include "Expr.h"
#include "ClassObject.h"

namespace expr 
{
ObjectValue* VarDeclaration::execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal )
{
    // var <var_name> : <type>
    if ( m_type != nullptr )
    {
        //todo: save type?
        if ( m_type->type == Identifier || m_type->type == IdentifierWithScope )
        {
            if ( m_type->lexeme == "Int" || m_type->lexeme == "Float" || m_type->lexeme == "String" )
            {

            }
            else
            {
                std::string_view identifierName;
                std::vector<std::string_view> namespaceSpec;
                parseIdentifierWithScope( *m_type, identifierName, namespaceSpec );

                auto* classDef = runtime.m_currentNamespace2->getClassDef( identifierName, namespaceSpec );
                if ( classDef == nullptr )
                {
                    throw runtime_ex3( "unknown type: " , *m_type );
                }

                assert( outValue.m_type == ot_null );
                outValue.m_type = ot_class_weak_ptr;
                new (&outValue.m_classWeakPtr) std::weak_ptr<ClassObject>();
                return &outValue;
            }
        }
        else if ( m_type->type != Int && m_type->type != Float && m_type->type != String )
        {
            throw runtime_ex3( "unknown type: " , *m_type );
        }
    }

    // var <var_name> = <init_value>
    if ( m_initValue == nullptr )
    {
        if ( m_objectRefType == is_weak_ref )
        {
            assert( outValue.m_type == ot_null );
            outValue.m_type = ot_class_weak_ptr;
            new (&outValue.m_classWeakPtr) std::weak_ptr<ClassObject>();
            return &outValue;
        }
        if ( m_objectRefType != is_weak_ref )
        {
            assert( outValue.m_type == ot_null );
            outValue.m_type = ot_class_shared_ptr;
            new (&outValue.m_classWeakPtr) std::shared_ptr<ClassObject>();
            return &outValue;
        }
        return nullptr;
    }

    ClassOrNamespace*   classDef = nullptr;

    // Cases:
    // var v = <SomeClassName>;
    // var v = <VariableName>;
    // var v = <SomeClassName>(...);
    // var v = <FuncName>(...);
    // var v = <Literal>;

    if ( m_initValue->type() == et_func_call )
    {
        auto* funcCall = (FunctionCall*) m_initValue;

        classDef = runtime.m_currentNamespace2->getClassDef( funcCall->m_functionName, funcCall->m_namespaceSpec );
        if ( classDef != nullptr )
        {
            ::createClassObject( outValue, runtime, isGlobal, *classDef );
        }
    }
    // var x = <another_var>
    else if ( m_initValue->type() == et_identifier )
    {
        auto* identifier = (IdentifierExpr*) m_initValue;

        classDef = runtime.m_currentNamespace2->getClassDef( identifier->m_name, identifier->m_namespaceSpec );
        if ( classDef != nullptr )
        {
            ::createClassObject( outValue, runtime, isGlobal, *classDef );
        }
        else
        {
            auto* value = runtime.m_currentNamespace2->getVarValue( identifier->m_name, identifier->m_namespaceSpec );

            if ( value == nullptr )
            {
                LOG( "Unknown idenifier or class name: " << m_initValue->m_token.lexeme )
                throw runtime_ex3( "Unknown idenifier or class name: ", m_initValue->m_token );
            }
        }
    }
    else if ( m_initValue->type() == et_int ||
             m_initValue->type()  == et_float ||
             m_initValue->type()  == et_string ||
             m_initValue->type()  == et_unary ||
             m_initValue->type()  == et_binary )
    {
        m_initValue->execute( outValue, runtime, isGlobal );
    }

    return nullptr;
}

ObjectValue* DotExpr::execute( ObjectValue& outValue, Runtime& runtime, bool isGlobal )
{
    LOG( "m_token.type == Dot" );
    // <m_expr2>.<m_expr>
    ObjectValue unused;
    ObjectValue* leftValueRef =  m_expr2->execute( unused, runtime, isGlobal );
    if ( leftValueRef->m_type != ot_class_shared_ptr && leftValueRef->m_type != ot_class_weak_ptr )
    {
        throw runtime_error( runtime, "cannot applay operator '.' ", m_token );
    }
    else
    {
        std::shared_ptr<ClassObject> sharedPtr;
        std::shared_ptr<ClassObject>* classObjRef = nullptr;

        if ( leftValueRef->m_type == ot_class_shared_ptr )
        {
            if ( not leftValueRef->m_classSharedPtr )
            {
                runtime.printRuntimeError( "variable is not initialised", *m_expr2 );
                exit(0);
            }
            classObjRef = &leftValueRef->m_classSharedPtr;
        }

        if ( leftValueRef->m_type == ot_class_weak_ptr )
        {
            sharedPtr = leftValueRef->m_classWeakPtr.lock();
            if ( not sharedPtr )
            {
                runtime.printRuntimeError( "weak reference value is unaccessible/expired", *m_expr2 );
                exit(0);
            }
            classObjRef = &sharedPtr;
        }

        if ( m_expr->type() == et_identifier )
        {
            std::shared_ptr<ClassObject>& classObj = *classObjRef;

            IdentifierExpr* id = (IdentifierExpr*) m_expr;
            runtime.dbgPrintLine( "todo++", *m_expr2 );
            if ( auto it = classObj->m_members.find( id->m_name ); it == classObj->m_members.end() )
            {
                auto message = "no member named '" + std::string(id->m_name) + "' in '" + std::string(classObj->m_definition->m_name) + "'";
                throw runtime_error( runtime, message , m_expr->m_token );
            }
            else
            {
                outValue = it->second;
                if ( outValue.m_type == ot_class_weak_ptr )
                {
                    if ( outValue.m_classWeakPtr.lock() )
                    {
                        return &it->second;
                    }
                    else
                    {
                        LOG("error");
                    }

                }
                return &it->second;
            }
        }
        else if ( m_expr->type() == et_func_call )
        {
            throw runtime_error( runtime, "todo", m_expr->m_token );
        }

    }
    return nullptr;
}

}
