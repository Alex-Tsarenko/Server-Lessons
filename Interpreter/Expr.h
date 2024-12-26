#pragma once

#include <forward_list>
#include <list>
#include <vector>
#include "Object.h"

#include "Token.h"
#include "Runtime.h"
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


inline std::string getLine( const char* text, int lineNumber )
{
    if ( lineNumber == 0)
    {
        return std::string( text, getLineEnd(text) );
    }
    
    int number = 0;
    for( const char* ptr = text; *ptr != 0; ptr++ )
    {
        if ( *ptr == 0 )
        {
            return "";
        }
        
        if ( *ptr == '\n' || *ptr == '\r' )
        {
            number++;
            while( *ptr == '\n' || *ptr == '\r' )
            {
                ptr++;
                if ( *ptr == 0 )
                {
                    return "";
                }
            }
            if ( number == lineNumber )
            {
                return std::string( ptr, getLineEnd(ptr) );
            }
        }
    }
}

namespace expr {

inline const Token& nilToken{ EndOfTokenType, "EndOfTokenType" };

enum ExpressionType
{
    et_unary,
    et_binary,
    et_identifier,
    et_int,
    et_float,
    et_string,
    et_func_call,
    
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
    virtual void evaluate2()
    {
        LOG("<Expression evaluate: NIL>");
    }
    
    virtual Object execute( Runtime& ) = 0;
};

struct HelpExpression: Expression
{
    HelpExpression( const Token& token ) : Expression(token) {};

    Object execute( Runtime& ) override { return gNullObject; }
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

inline void evalPrint( const std::string& lexeme )
{
    for( int i=0; i<gEvaluateOffset; i++ )
    {
        LOGX(' ')
    }
    LOG(lexeme)
}

inline void evalPrintOp2( const std::string& lexeme )
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


struct ExpressionList: public Expression
{
    std::list<Expression*>  m_list;
    
    virtual Object execute( Runtime& runtime ) override
    {
        for( auto* expr: m_list )
        {
            expr->execute( runtime );
        }
        return gNullObject;
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
    
    virtual Object execute( Runtime& runtime ) override
    {
        for( auto* expr: m_list )
        {
            Object value = expr->execute( runtime );
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
    enum Operator { plus, minus, negation, star, ampersand };
    
    virtual enum ExpressionType type() override { return et_unary; }

    Operator       m_op;
    Expression*    m_expr = nullptr;

    UnaryExpression( const Token& token ) : Expression(token) {}

    virtual void evaluate() override
    {
        evalPrint( m_token.lexeme );
        evalPrint( m_expr );
    }
    virtual void evaluate2() override
    {
        LOGX( "(" << m_token.lexeme << " " );
        m_expr->evaluate2();
        LOGX( ")");
    }
};

// Binary expression with unary operator
struct BinaryOpExpression: public Expression
{
    BinaryOpExpression( const Token& token ) : Expression(token) {}
    Expression*  m_expr  = nullptr;
    Expression*  m_expr2 = nullptr;
    
    virtual enum ExpressionType type() override { return et_binary; }

    virtual void evaluate() override
    {
        evalPrintOp2( m_token.lexeme );
        evalPrint( m_expr2 );
        evalPrint( m_expr );
    }
    
    virtual void evaluate2() override
    {
        LOGX( "(" );
        m_expr2->evaluate2();
        LOGX( " " << m_token.lexeme << " " );
        m_expr->evaluate2();
        LOGX( ")");
    }

};

struct ExpressionVarDecl: public Expression
{
    ExpressionVarDecl( const Token& token ) : Expression(token), m_identifierName(token.lexeme) {}
    
    std::string   m_identifierName;
    Expression*   m_initValue;
    //TODO: Value
};


// Binary expression with unary operator
struct FunctionCall: public Expression
{
    FunctionCall( const Token& token ) : Expression(token), m_functionName(token.lexeme) {}
    
    virtual enum ExpressionType type() override { return et_func_call; }
    
    std::string                 m_functionName;
    std::vector<Expression*>    m_parameters;
    
    virtual void evaluate2() override
    {
        LOGX( m_token.lexeme << "(" );
        for( auto& parameter: m_parameters )
        {
            LOGX( " ");
            parameter->evaluate2();
            LOGX( ",");
        }
        LOGX( ")");
    }
    
    Object execute( Runtime& runtime ) override
    {
        if ( auto it = runtime.m_funcMap.find( m_functionName ); it == runtime.m_funcMap.end() )
        {
            RUNTIME_EX2( std::string("Undefined function: ") + m_functionName );
        }
        else
        {
            auto paramIt = m_parameters.begin();
            FuncDefinition* f = it->second;
            for( const auto& arg: f.m_argList )
            {
                if ( paramIt != m_parameters.end() )
                {
                    paramValue = paramIt->execute( runtime );
                    arg.m_name;
                    //...
                }
            }
            //m_parameters
        }
    }

};

struct Argument
{
    std::string m_name;
    std::string m_type;
    std::string m_defaultValue;
};

struct Identifier : public Expression
{
    Identifier( const Token& token ) : Expression(token), m_name(token.lexeme) {}

    const std::string&  m_name;
    std::string         m_type;
    
    virtual enum ExpressionType type() override { return et_identifier; }
    
    virtual void evaluate() override
    {
        evalPrint( m_name );
    }

    virtual void evaluate2() override
    {
        LOGX( "'" << m_token.lexeme );
    }

};

struct IntNumber : public Expression
{
    int64_t         m_value;
    
    IntNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stol( m_token.lexeme );
    }
    
    virtual enum ExpressionType type() override { return et_int; }

    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void evaluate2() override
    {
        LOGX( m_value );
    }
};

struct FloatNumber : public Expression
{
    double          m_value;
    
    FloatNumber( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = std::stod( m_token.lexeme );
    }
    
    virtual enum ExpressionType type() override { return et_float; }

    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void evaluate2() override
    {
        LOGX( m_value );
    }
};

struct String : public Expression
{
    std::string m_value;
    
    String( const Token& lexeme ) : Expression(lexeme)
    {
        m_value = m_token.lexeme;
    }
    
    String( const Token& lexeme, const char* begin, const char* end ) : Expression(lexeme)
    {
        m_value = std::string( begin, end );
    }
    
    virtual enum ExpressionType type() override { return et_string; }

    virtual void evaluate() override
    {
        evalPrint( m_value );
    }
    
    virtual void evaluate2() override
    {
        LOGX( m_value );
    }
};

struct FuncDefinition : public Expression
{
    std::string             m_name;
    std::vector<Argument>   m_argList;
    ExpressionList          m_body;
    
    Object execute( Runtime& runtime ) override
    {
        auto result = runtime.m_funcMap.emplace({m_functionName, this});
        if ( result.second )
        {
            return gNullObject;
        }

        // Insertion failed, meaning m_functionName already exists
        RUNTIME_EX2( std::string("Double function declaration: ") + m_functionName );
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
};

struct Print : public Expression
{
    ExpressionList  m_printParameters;
};

}
