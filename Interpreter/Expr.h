#pragma once

#include <forward_list>
#include <list>
#include <vector>
#include <assert.h>

#include "ObjectValue.h"

#include "Token.h"
#include "Runtime.h"
#include "Error.h"
#include "Logs.h"

inline const char* getLineEnd( const char* text )
{
    for( const char* ptr = text; ; ptr++ )
    {
        if ( *ptr == '\n' || *ptr == '\r' || *ptr == 0 )
        {
            return ptr;
        }
    }
}


inline std::string_view getLine( const char* text, int lineNumber )
{
    if ( lineNumber == 0)
    {
        return std::string_view( text, getLineEnd(text) );
    }
    
    int number = 0;
    for( const char* ptr = text; *ptr != 0; ptr++ )
    {
        if ( *ptr == 0 )
        {
            return "<eof>";
        }
        
        if ( *ptr == '\n' )
        {
            number++;

            //todo '\r'
            if ( number == lineNumber )
            {
                return std::string_view( ptr+1, getLineEnd(ptr+1) );
            }
        }
    }
}

struct Namespace;

namespace expr {

void initIdentifierWithScope( const Token& token, std::string_view& identifierName, std::vector<std::string_view>& namespaceSpec );

struct VarDeclaration;

struct using_of_uninitialized_variable: public std::runtime_error
{
    Namespace*      m_namespace;
    VarDeclaration* m_varDecl;
};

struct unknow_variable: public std::runtime_error
{
};

struct unknow_function: public std::runtime_error
{
};


inline const Token& nilToken{ NilToken, "NilToken" };

enum ExpressionType
{
    et_unary,
    et_binary,
    et_identifier,
    et_int,
    et_float,
    et_string,
    et_assignment,
    et_func_call,
    et_namespace_op,
    et_return,

    et_undefined
};

struct Expression
{
    const Token& m_token;

    Expression() : m_token(nilToken) {
        
    };
    Expression( const Token& token ) : m_token(token) {};
 
    virtual ~Expression() {
        
    };
    

    virtual enum ExpressionType type() { return et_undefined; }
    
    virtual void evaluate()
    {
        LOG("<Expression evaluate: NIL>");
    }
    virtual void printExpr()
    {
        LOG("<Expression evaluate: NIL>");
    }
    
    virtual ObjectValue execute( Runtime&, bool isGlobal ) = 0;
};

#include "ClassOrNamespace.h"


struct HelpExpression: Expression
{
    HelpExpression( const Token& token ) : Expression(token) {};

    ObjectValue execute( Runtime&, bool isGlobal ) override { return gNullObject; }
};

inline int gEvaluateOffset = 0;

inline void evalPrint( Expression* ptr )
{
    gEvaluateOffset += 2;
    
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    if ( ptr == nullptr )
    {
        LOGX("<expr:nullptr>")
    }
    else
    {
        ptr->evaluate();
    }
    
    gEvaluateOffset -= 2;
}

inline void evalPrint( const std::string_view& lexeme )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(lexeme)
}

inline void evalPrintOp2( const std::string_view& lexeme )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(lexeme)
}

inline void evalPrint( int64_t number )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(number)
}

inline void evalPrint( double number )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(number)
}


// Block
struct ExpressionList: public Expression
{
    std::list<Expression*>  m_list;
    
    virtual ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue result;
        for( auto& statement: m_list )
        {
            LOG("statement->m_token: " << statement->m_token.lexeme )
            result = statement->execute( runtime, isGlobal );
            if ( result.m_isReturned )
            {
                return result;
            }

            if ( statement->m_token.type == Identifier )
            {
                auto pair = runtime.m_localVarStack.back().emplace( statement->m_token.lexeme, result );
                if ( not pair.second )
                {
                    throw runtime_ex( "duplicate local variable: " + std::string(statement->m_token.lexeme) );
                }
            }
        }
        return result;
    }
};

struct PrintFuncCall: public Expression
{
    bool m_withNewLine = false;
    
    PrintFuncCall( std::vector<Expression*>&&  list )
    {
        m_list = std::move(list);
    }
    std::vector<Expression*>  m_list;
    
    virtual ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        for( auto* expr: m_list )
        {
            LOG("PrintFunc item: " << (void*)expr )
            ObjectValue value = expr->execute( runtime, isGlobal );
            LOG("PrintFunc item: " << value.pstring() )

            value.toStream( std::cout );
        }
       
        if ( m_withNewLine )
        {
            std::cout << std::endl;
        }
        return gNullObject;
    }
};

// Unary expression with unary operator
struct UnaryExpression: public Expression
{
    enum Operator { plus, minus, negotiaton, star, ampersand };
    
    virtual enum ExpressionType type() override { return et_unary; }

    Operator       m_op;
    Expression*    m_expr = nullptr;

    UnaryExpression( const Token& token ) : Expression(token) {}

    virtual void evaluate() override
    {
        evalPrint( m_token.lexeme );
        evalPrint( m_expr );
    }
    virtual void printExpr() override
    {
        LOGX( "(" << m_token.lexeme << " " );
        m_expr->printExpr();
        LOGX( ")");
    }
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue value = m_expr->execute( runtime, isGlobal );

        switch (m_op) {
            case plus:
                break;
            case minus:
                if ( value.m_type == ot_int )
                {
                    value.m_intValue = -value.m_intValue;
                    break;
                }
                else if ( value.m_type == ot_double )
                {
                    value.m_doubleValue = -value.m_doubleValue;
                    break;
                }
                else if ( value.m_type == ot_bool )
                {
                    RUNTIME_EX3( "Cannot apply '-' to bool value: ", m_token );
                }
                else if ( value.m_type == ot_string )
                {
                    RUNTIME_EX3( "Cannot apply '-' to string value: ", m_token );
                }
            case negotiaton:
                if ( value.m_type == ot_bool )
                {
                    value.m_boolValue = -value.m_boolValue;
                    break;
                }
                else if ( value.m_type == ot_int )
                {
                    RUNTIME_EX3( "Cannot apply '!' to int value: ", m_token );
                }
                else if ( value.m_type == ot_double )
                {
                    RUNTIME_EX3( "Cannot apply '!' to double value: ", m_token );
                }
                else if ( value.m_type == ot_string )
                {
                    RUNTIME_EX3( "Cannot apply '-' to string value: ", m_token );
                }
                else if ( value.m_type == ot_null )
                {
                    RUNTIME_EX3( "Cannot apply '-' to null: ", m_token );
                }
            case star:
                RUNTIME_EX3( "'*' not supported: ", m_token );
            case ampersand:
                RUNTIME_EX3( "'&' not supported: ", m_token );

            default:
                break;
        }
        return value;
    }
};

// Binary expression with unary operator
struct BinaryOpExpression: public Expression
{
    BinaryOpExpression( const Token& token ) : Expression(token)
    {
    }
    Expression*  m_expr  = nullptr;
    Expression*  m_expr2 = nullptr;
    
    virtual enum ExpressionType type() override { return et_binary; }

    virtual void evaluate() override
    {
        evalPrintOp2( m_token.lexeme );
        evalPrint( m_expr2 );
        evalPrint( m_expr );
    }
    
    virtual void printExpr() override
    {
        LOGX( "(" );
        m_expr2->printExpr();
        LOGX( " " << m_token.lexeme << " " );
        m_expr->printExpr();
        LOGX( ")");
    }

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        ObjectValue value1 = m_expr->execute( runtime, isGlobal );
        if ( value1.isNull() )
        {
            if ( m_expr->type() == et_identifier )
            {
                RUNTIME_EX3( "undefined variable: ", m_expr->m_token )
            }
            RUNTIME_EX3( "unexpected null value: ", m_expr->m_token )
        }

        ObjectValue value2 = m_expr2->execute( runtime, isGlobal );
        if ( value2.isNull() )
        {
            if ( m_expr2->type() == et_identifier )
            {
                RUNTIME_EX3( "undefined variable: ", m_expr2->m_token )
            }
            RUNTIME_EX3( "unexpected null value: ", m_expr2->m_token )
        }

        if ( value1.m_type == ot_int && value2.m_type == ot_int )
        {
#define RETURN_INT_VALUE_OF(sign) \
    ObjectValue value;\
    value.m_type = ot_int;\
    value.m_intValue = value1.m_intValue sign value2.m_intValue;\
    return value;

            if ( m_token.type == Plus )
            {
                RETURN_INT_VALUE_OF(+)
            }
            if ( m_token.type == Minus )
            {
                RETURN_INT_VALUE_OF(-)
            }
            if ( m_token.type == Slash )
            {
                RETURN_INT_VALUE_OF(/)
            }
            if ( m_token.type == Star )
            {
                RETURN_INT_VALUE_OF(*)
            }
        }
        if ( (value1.m_type == ot_int || value1.m_type == ot_double) || (value2.m_type == ot_int || value2.m_type == ot_double) )
        {
#define RETURN_DOUBLE_VALUE_OF(sign) \
    ObjectValue value;\
    value.m_type = ot_double;\
    value.m_doubleValue = value1.doubleValue() sign value2.doubleValue();\
    return value;

            if ( m_token.type == Plus )
            {
                RETURN_DOUBLE_VALUE_OF(+)
            }
            if ( m_token.type == Minus )
            {
                RETURN_DOUBLE_VALUE_OF(-)
            }
            if ( m_token.type == Slash )
            {
                RETURN_DOUBLE_VALUE_OF(/)
            }
            if ( m_token.type == Star )
            {
                RETURN_DOUBLE_VALUE_OF(*)
            }
        }
        LOG( "????");
    }
};

struct VarDeclaration: public Expression
{
    VarDeclaration( const Token& token, const std::string_view& varType ) : Expression(token), m_identifierName(token.lexeme), m_type(varType) {}

    bool               m_isPrivate = false;
    bool               m_isStatic = false;

    std::string_view   m_identifierName;
    std::string_view   m_type;
    Expression*        m_initValue;
    //TODO: Value
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        if ( m_initValue != nullptr )
        {
            expr::ClassDefinition* classDef = nullptr;

            if ( m_initValue->m_token.type == Identifier )
            {
                std::vector<std::string_view> namespaceSpec;
                classDef = runtime.m_currentNamespace2->getClassDef( m_initValue->m_token.lexeme, namespaceSpec );
            }
            else if ( m_initValue->m_token.type == IdentifierWithScope )
            {
                std::string_view identifierName;
                std::vector<std::string_view> namespaceSpec;
                initIdentifierWithScope( m_initValue->m_token, identifierName, namespaceSpec );

                classDef = runtime.m_currentNamespace2->getClassDef( identifierName, namespaceSpec );
            }

            if ( classDef != nullptr )
            {
//                ObjectValue value = classDef .....;
//                return value;
            }

            ObjectValue value = m_initValue->execute(runtime, isGlobal);
            return value;
        }

        return gNullObject;
    }
};

struct Argument
{
    std::string_view m_name;
    std::string_view m_type;
//    std::string_view m_defaultValue;
};


inline void initIdentifierWithScope( const Token& token, std::string_view& identifierName, std::vector<std::string_view>& namespaceSpec )
{
    LOG( "??: " << token.lexeme );
    bool scopeOp = false;
    auto end = token.lexeme.data() + token.lexeme.size();
    for( const char* ptr = token.lexeme.data(); ptr < end; ptr++ )
    {
        if ( *ptr == ':' )
        {
            assert( *(ptr+1) == ':' );
            assert( !scopeOp );
            ptr++;
            if ( namespaceSpec.empty() )
            {
                namespaceSpec.push_back( std::string_view{ ptr, 0 } );
            }
            scopeOp = true;
        }
        else
        {
            assert( scopeOp || namespaceSpec.empty() );

            auto* begin = ptr;
            while( ptr < end and ( isalpha(*ptr) or *ptr == '_' or isdigit(*ptr) ) )
            {
                ptr++;
            }
            assert( begin < ptr );

            if ( ptr==end )
            {
                identifierName = std::string_view{ begin, ptr };
            }
            else
            {
                namespaceSpec.push_back( std::string_view{ begin, ptr } );
                ptr--;
            }

            scopeOp = false;
        }
    }
}

struct Identifier : public Expression
{
    std::string_view     m_name;
    std::string_view     m_type;

    std::vector<std::string_view>   m_namespaceSpec;

    Identifier( const Token& token ) : Expression(token)
    {
        if ( token.type == IdentifierWithScope )
        {
            initIdentifierWithScope( token, m_name, m_namespaceSpec );
        }
        else
        {
            m_name = token.lexeme;
        }
    }


    virtual enum ExpressionType type() override { return et_identifier; }
    
    virtual void evaluate() override
    {
        evalPrint( m_name );
    }

    virtual void printExpr() override
    {
        LOGX( "'" << m_token.lexeme );
    }

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        LOG( "execute var: " << m_name )

        if ( isGlobal )
        {
            LOG( "execute var ???: " << m_name )
            if ( auto it = runtime.m_currentNamespace2->m_variableMap.find(m_name); it != runtime.m_currentNamespace2->m_variableMap.end() )
            {
                if ( auto it = runtime.m_currentNamespace2->m_variableValueMap.find(m_name); it != runtime.m_currentNamespace2->m_variableValueMap.end() )
                {
                    return it->second;
                }
            }
            else
            {
                auto* oldCurrentNamespace = runtime.m_currentNamespace2;
                for(;;)
                {
                    LOG( "-- runtime.m_currentNamespace : " << runtime.m_currentNamespace2->m_name )
                    if ( runtime.m_currentNamespace2 = runtime.m_currentNamespace2->m_parent; runtime.m_currentNamespace2 == nullptr )
                    {
                        LOG( "-- break: runtime.m_currentNamespace"  )
                        break;
                    }
                    LOG( "-- after runtime.m_currentNamespace : " << runtime.m_currentNamespace2->m_name )
                    if ( auto it = runtime.m_currentNamespace2->m_variableValueMap.find(m_name); it != runtime.m_currentNamespace2->m_variableValueMap.end() )
                    {
                        runtime.m_currentNamespace2 = oldCurrentNamespace;
                        return it->second;
                    }
                }
                runtime.m_currentNamespace2 = oldCurrentNamespace;

                throw runtime_error( runtime, "unknown variable: '" + std::string(m_token.lexeme) + "'", m_token );
            }
        }
        else
        {
            for( auto localVarMapIt = runtime.m_localVarStack.rbegin();
                 localVarMapIt != runtime.m_localVarStack.rend();
                 localVarMapIt++ )
            {
                if ( auto it = localVarMapIt->find(m_name); it != localVarMapIt->end() )
                {
                    return it->second;
                }
            }
        }

        if ( auto* value = runtime.m_currentNamespace2->getVarValue(m_name, m_namespaceSpec); value != nullptr )
        {
            return *value;
        }
        else
        {
            throw runtime_error( runtime, "unknown variable: '" + std::string(m_token.lexeme) + "'", m_token );
        }

        return gNullObject;
    }

};

struct IntNumber : public Expression
{
    int64_t         m_value;
    
    IntNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stol( std::string(m_token.lexeme) );
    }
    
    virtual enum ExpressionType type() override { return et_int; }

    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void printExpr() override
    {
        LOGX( m_value );
    }
    
    ObjectValue execute( Runtime&, bool isGlobal ) override
    {
        ObjectValue value;
        value.m_type = ot_int;
        value.m_intValue = m_value;
        return value;
    }
};

struct FloatNumber : public Expression
{
    double          m_value;
    
    FloatNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stod( std::string(m_token.lexeme) );
    }
    
    virtual enum ExpressionType type() override { return et_float; }
    
    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void printExpr() override
    {
        LOGX( m_value );
    }
    
    ObjectValue execute( Runtime&, bool isGlobal ) override
    {
        ObjectValue value;
        value.m_type = ot_double;
        value.m_doubleValue = m_value;
        return value;
    }
};

struct String : public Expression
{
    std::string_view m_value;

    String( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = m_token.lexeme;
    }
    
    String( const Token& lexeme, const char* begin, const char* end ) : Expression(lexeme)
    {
        m_value = std::string_view( begin, end );
    }
    
    virtual enum ExpressionType type() override { return et_string; }

    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void printExpr() override
    {
        LOGX( m_value );
    }
    
    ObjectValue execute( Runtime&, bool isGlobal ) override
    {
        ObjectValue value;
        value.m_type = ot_string;
        value.m_stringValue = new std::string{ m_value };
        return value;
    }

};

struct FuncDefinition : public Expression
{
    bool                    m_isPrivate = false;
    bool                    m_isStatic = false;
    std::string_view        m_name;
    std::vector<Argument>   m_argList;
    ExpressionList          m_body;
    ClassOrNamespace*       m_whereFuctionWasDefined = nullptr;

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        auto result = runtime.m_topLevelNamespace.m_functionMap.emplace( m_name, this);
        if ( result.second )
        {
            // we added function
            return gNullObject;
        }

        // Insertion failed, meaning m_functionName already exists
        RUNTIME_EX2( std::string("Double function declaration: ") + std::string(m_name) );
    }
};

struct ConstructorInfo: FuncDefinition
{
    bool                        m_isPrivate;
    std::vector<Expression*>    m_baseClassInitList;
};

struct ClassDefinition : public Expression
{
    struct BaseClassInfo
    {
        bool             m_isPrivate;
        std::string_view m_name;
    };
    
    struct ConstructorInfo: FuncDefinition
    {
        bool                        m_isPrivate;
        std::vector<Expression*>    m_baseClassInitList;
    };

    std::vector<ConstructorInfo*>   m_constuctors;

    // members
    std::vector<BaseClassInfo>      m_baseClasses;
    
    std::map<std::string_view,ClassDefinition*> m_innerClasses;

    ClassDefinition( const Token& lexeme ) : Expression(lexeme)/*, m_name(lexeme.lexeme)*/ {}
    ClassDefinition( const Token& lexeme, const std::string_view& outerClassName, const std::vector<std::string_view>& outerClasses  )
        : Expression(lexeme)
    {
    }

    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
    }
};

struct AssignmentStatement: public Expression
{
    std::string_view            m_varName;
    expr::Expression*           m_expr = nullptr;
    
    AssignmentStatement( const Token& token ) : Expression(token), m_varName(token.lexeme) {}
    
    virtual enum ExpressionType type() override { return et_assignment; }
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        assert("todo");
    }
};

// Binary expression with unary operator
struct FunctionCall: public Expression
{
    std::string_view            m_functionName;
    std::vector<Expression*>    m_parameters;

    std::vector<std::string_view>   m_namespaceSpec;

    FunctionCall( const Token& token ) : Expression(token)
    {
        if ( token.type == IdentifierWithScope )
        {
            initIdentifierWithScope( token, m_functionName, m_namespaceSpec );
        }
        else
        {
            m_functionName = token.lexeme;
        }
    }

    virtual enum ExpressionType type() override { return et_func_call; }
    
    virtual void printExpr() override
    {
        LOGX( m_token.lexeme << "(" );
        bool atFirst = true;
        for( auto& parameter: m_parameters )
        {
            if ( atFirst )
            {
                atFirst = false;
            }
            else
            {
                LOGX( ",");
            }
            LOGX( " ");
            parameter->printExpr();
        }
        LOGX( ")");
    }
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        FuncDefinition* funcDef = nullptr;

        // ::func()
        funcDef = runtime.m_currentNamespace2->getFunctionDef( m_functionName, m_namespaceSpec );

        if ( funcDef == nullptr )
        {
            throw runtime_error( runtime, "unknow function: ", m_token );
        }

        auto oldCurrentNamespace = runtime.m_currentNamespace2;
        runtime.m_currentNamespace2 = funcDef->m_whereFuctionWasDefined;
        //{
            auto result = FunctionCall::doExecute( runtime, funcDef, isGlobal );
        //}
        runtime.m_currentNamespace2 = oldCurrentNamespace;

        return result;
    }

    ObjectValue doExecute( Runtime& runtime, FuncDefinition* fDefinition, bool isGlobal )
    {
        runtime.m_localVarStack.push_back({});

        auto paramIt = m_parameters.begin();
        for( const auto& arg: fDefinition->m_argList )
        {
            if ( paramIt != m_parameters.end() )
            {
                ObjectValue paramValue = (*paramIt)->execute( runtime, isGlobal );
                LOG("execute function: " << m_functionName << ": arg: " << arg.m_name << " = " << paramValue.pstring() )
                auto result = runtime.m_localVarStack.back().emplace( arg.m_name, paramValue );
                paramIt++;
            }
            else
            {
                auto result = runtime.m_localVarStack.back().emplace( arg.m_name, ObjectValue{} );
            }
        }

        auto result = fDefinition->m_body.execute( runtime, isGlobal );
        result.m_isReturned = 0;

        runtime.m_localVarStack.pop_back();
        return result;
    }

};

struct If : public Expression
{
    Expression*             m_condition;
    ExpressionList          m_yesBlock;
    ExpressionList          m_noBlock;
};

//struct VarDefExpression : public Expression
//{
//    std::string     m_name;
//    std::string     m_type;
//    Expression*     m_value;
//};

struct For : public Expression
{
    std::string     m_iteratorName;
    Expression*     m_condition;
    Expression*     m_getNext;
};

struct Return : public Expression
{
    Expression*     m_returnValue;
    
    Return( Expression* returnValue ) : m_returnValue(returnValue) {}
    
    ObjectValue execute( Runtime& runtime, bool isGlobal ) override
    {
        if ( m_returnValue != nullptr )
        {
            auto returnValue = m_returnValue->execute( runtime, isGlobal );
            returnValue.m_isReturned = 0xff;
            return returnValue;
        }
        return gNullObject;
    }
};

struct Print : public Expression
{
    ExpressionList  m_printParameters;
};

}

