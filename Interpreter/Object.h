#pragma once

#include <string>
#include <iostream>

struct runtime_ex : std::runtime_error
{
    runtime_ex( const std::string& message ) : std::runtime_error(message) {}
};

#define RUNTIME_EX throw runtime_ex( std::string("runtime exception: in ") + __func__ );
#define RUNTIME_EX2(message) throw runtime_ex( message );

enum ObjectType { ot_null, ot_bool, ot_int, ot_double, ot_string, };

struct Object
{
    ObjectType m_type;
    union {
        bool    m_boolValue;
        int     m_intValue;
        double  m_doubleValue;
        std::string* m_stringValue;
    };
    
    Object()
    {
        m_type = ot_null;
        m_stringValue = nullptr;
    }
    ~Object()
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
    
    int intValue() const
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


#define gNullObject Object{}


