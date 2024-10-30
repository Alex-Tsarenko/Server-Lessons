#pragma once

#include <string>

class ObjectBase
{
public:
    virtual bool        boolValue() const = 0;
    virtual int         intValue() const = 0;
    virtual double      doubleValue() const = 0;
    virtual std::string stringValue() const = 0;

    virtual void        setBool( bool newValue ) = 0;
    virtual void        setInt( int newValue ) = 0;
    virtual bool        setDouble( double newValue ) = 0;
    virtual bool        setString( const std::string& newValue ) = 0;
};

class IntObject : public ObjectBase
{
    int m_value;

protected:
    bool        boolValue() const   { return m_value != 0; }
    int         intValue() const    { return m_value; }
    double      doubleValue() const { return static_cast<double>( m_value ); }
    std::string stringValue() const { return std::to_string( m_value ); }

    void        setBool( bool newValue )     { m_value = newValue ? -1 : 0; }
    void        setInt( int newValue )       { m_value = newValue; }
    
    bool        setDouble( double newValue )
    {
        double intPart;
        double fractPart = modf( newValue, &intPart );
        if ( fractPart == 0 )
        {
            m_value = (int) intPart;
            return true;
        }
        return false;
    }
    
    bool        setString( const std::string& newValue ) try
    {
        m_value = std::stoi( newValue.c_str() );
        return true;
    }
    catch (...)
    {
        return false;
    }
};
