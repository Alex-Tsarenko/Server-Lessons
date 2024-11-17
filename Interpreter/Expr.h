#pragma once

#include <forward_list>
#include <list>
#include <vector>
#include "Object.h"

class Executor;

const char* getLineEnd( const char* text )
{
    for( const char* ptr = text; ; ptr++ )
    {
        if ( *ptr == '\n' || *ptr == '\r' || *ptr == 0 )
        {
            return ptr;
        }
    }
}


std::string getLine( const char* text, int lineNumber )
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

struct Expression
{
    //virtual ~Expression() = default;
};

//using ExpressionPtr = Expression*;

// It is used for auto-deleting of "Expression"
struct ExpressionPtr
{
    ExpressionPtr( Expression* expr ) : m_expr(expr) {}

    ExpressionPtr( ExpressionPtr&& exprPtr ) : m_expr(exprPtr.m_expr)
    {
        exprPtr.m_expr = nullptr;
    }
    
    ExpressionPtr& operator=( ExpressionPtr&& exprPtr )
    {
        m_expr = exprPtr.m_expr;
        if ( this != &exprPtr )
        {
            exprPtr.m_expr = nullptr;
        }
    }

    ~ExpressionPtr() {
        delete m_expr;
    }
    
    Expression* m_expr = nullptr;
};

// Unary expression with unary operator
struct UnaryExpression: public Expression
{
    enum Operator { plus, minus, negation, star, ampersand };
    
    Operator       m_op;
    ExpressionPtr  m_expr;
};

// Binary expression with unary operator
struct BinaryExpression: public Expression
{
    enum Operator { plus, minus, star, devide, equal, not_equal, less, less_or_equal, more, more_or_equal, left_shift, right_shift, negation, logic_and, logic_or, bit_and, bit_or };
    
    Operator       m_op;
    ExpressionPtr  m_expr;
    ExpressionPtr  m_expr2;
};

// Binary expression with unary operator
//: public Expression
struct ExpressionVarDecl
{
    std::string     m_identifierName;
    ExpressionPtr   m_expr = nullptr;
};


// Binary expression with unary operator
struct FunctionCall: public Expression
{
    std::string                 m_functionName;
    std::vector<ExpressionPtr>  m_parameters;
};

struct ExpressionList: public Expression
{
    std::list<ExpressionPtr> m_list;
};

struct Argument
{
    std::string m_name;
    std::string m_type;
    std::string m_defaultValue;
};

struct Func : public Expression
{
    std::string             m_name;
    std::vector<Argument>   m_argList;
    ExpressionList          m_body;
};

struct If : public Expression
{
    ExpressionPtr           m_condition;
    ExpressionList          m_yesBlock;
    ExpressionList          m_noBlock;
};

struct VariableDefinition : public Expression
{
    std::string     m_name;
    std::string     m_type;
    ExpressionPtr   m_value;
};

struct For : public Expression
{
    std::string     m_iteratorName;
    ExpressionPtr   m_condition;
    ExpressionPtr   m_getNext;
};

struct Return : public Expression
{
    ExpressionPtr   m_returnValue;
};

struct Print : public Expression
{
    ExpressionList  m_printParameters;
};

}
