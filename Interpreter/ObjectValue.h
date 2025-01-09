#pragma once

#include <string>
#include <iostream>

struct runtime_ex : std::runtime_error
{
    runtime_ex( const std::string& message ) : std::runtime_error(message) {}
};

struct Token;
struct runtime_ex3 : std::runtime_error
{
    const Token& m_token;
    runtime_ex3( const std::string& message, const Token& tokenRef ) : std::runtime_error(message), m_token(tokenRef) {}
};

#define RUNTIME_EX throw runtime_ex( std::string("runtime exception: in ") + __func__ );
#define RUNTIME_EX2(message) throw runtime_ex( message );
#define RUNTIME_EX3(message,token) throw runtime_ex3( message,token );

enum ObjectType : uint8_t { ot_null, ot_bool, ot_int, ot_double, ot_string };

struct ObjectValue
{
    ObjectType m_type;
    uint8_t    m_isReturned = 0;
    
    union {
        bool        m_boolValue;
        uint64_t    m_intValue;
        double      m_doubleValue;
        std::string* m_stringValue;
    };
    
    ObjectValue()
    {
        m_type = ot_null;
        m_stringValue = nullptr;
    }
    ~ObjectValue()
    {
        if ( m_type == ot_string )
        {
            delete m_stringValue;
        }
    }

    void toStream( std::ostream& stream )
    {
        switch( m_type )
        {
            case ot_null: stream << "<null>"; break;
            case ot_bool: stream << m_boolValue; break;
            case ot_int: stream << m_intValue; break;
            case ot_double: stream << m_doubleValue; break;
            case ot_string: stream << *m_stringValue; break;
        }
    }
    
    bool isNull() const { return m_type == ot_null; };
    
    bool boolValue() const
    {
        if ( m_type == ot_bool )
        {
            return m_boolValue;
        }
        RUNTIME_EX;
    };
    
    uint64_t intValue() const
    {
        if ( m_type == ot_int )
        {
            return m_intValue;
        }
        RUNTIME_EX;
    };
    
    double doubleValue() const
    {
        if ( m_type == ot_int )
        {
            return m_intValue;
        }
        if ( m_type == ot_double )
        {
            return m_doubleValue;
        }
        RUNTIME_EX;
    };

    const std::string& stringValue() const
    {
        if ( m_type == ot_string )
        {
            return *m_stringValue;
        }
        RUNTIME_EX;
    };


    void setBool( bool newValue )
    {
        if ( m_type == ot_string )
        {
            delete m_stringValue;
        }
        m_type = ot_bool;
        m_boolValue = newValue;
    }

    void setInt( int newValue )
    {
        if ( m_type == ot_string )
        {
            delete m_stringValue;
        }
        m_type = ot_int;
        m_intValue = newValue;
    }

    void setDouble( int newValue )
    {
        if ( m_type == ot_string )
        {
            delete m_stringValue;
        }
        m_type = ot_double;
        m_doubleValue = newValue;
    }

    void setString( const std::string& newValue )
    {
        if ( m_type == ot_string )
        {
            *m_stringValue = newValue;
        }
        else
        {
            m_type = ot_string;
            m_stringValue = new std::string{newValue};
        }
    }
};


#define gNullObject ObjectValue{}


