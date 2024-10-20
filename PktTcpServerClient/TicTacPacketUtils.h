#pragma once

#include <iostream>
#include <vector>

namespace tic_tac {

class PacketReader
{
    const uint8_t* m_bufferPtr;
    const uint8_t* m_bufferEnd;
    
public:
    PacketReader( const uint8_t* bufferPtr, const uint8_t* bufferEnd )
    : m_bufferPtr(bufferPtr),
    m_bufferEnd(bufferEnd) {}
    
    template<typename First, typename ...Args>
    void operator()( First& first, Args&... tail )
    {
        read( first );
        (*this)( tail... );
    }
    
    void operator()() {}
    
    void read( bool& outValue )
    {
        if ( m_bufferPtr + 1 > m_bufferEnd )
        {
            throw std::runtime_error("Buffer length too small (bool)" );
        }
        
        outValue = (m_bufferPtr[0] != 0);
        m_bufferPtr ++;
    }
    
    void read( uint16_t& outNumber )
    {
        if ( m_bufferPtr + 2 > m_bufferEnd )
        {
            throw std::runtime_error("Buffer length too small (uint16_t)" );
        }
        
        outNumber = m_bufferPtr[0] | (m_bufferPtr[1] << 8);
        m_bufferPtr += 2;
    }
    
    void read( std::string& outString )
    {
        uint16_t stringLength;
        read( stringLength );
        
        if ( m_bufferPtr + stringLength > m_bufferEnd )
        {
            throw std::runtime_error("Buffer length too small (std::string); string length: " + std::to_string(stringLength) );
        }
        
        if ( stringLength == 0 )
        {
            assert( outString.empty() );
        }
        else
        {
            outString.resize( stringLength );
            std::memcpy( const_cast<char*>(outString.c_str()), m_bufferPtr, stringLength );
            m_bufferPtr += stringLength;
        }
    }
    
    template<typename T>
    void read( std::vector<T>& outVector )
    {
        uint16_t vectorLength;
        read( vectorLength );
        
        outVector.reserve( vectorLength );
        
        for( int i=0; i<vectorLength; i++ )
        {
            outVector.emplace_back(T{});
            read( outVector.back() );
        }
    }
    
    template<typename T>
    void read( T& tObject )
    {
        tObject.fields( *this );
    }
};

class PacketWriter
{
    uint8_t* m_bufferPtr;
    uint8_t* m_bufferEnd;
    
public:
    PacketWriter( uint8_t*  bufferPtr,
                 size_t     tcpPacketSize )
      :
        m_bufferPtr( bufferPtr ),
        m_bufferEnd( bufferPtr+tcpPacketSize )
    {
#ifdef DEBUG
        assert( tcpPacketSize <= 0xFFFF );
#endif
    }
    
    template<typename First, typename ...Args>
    void operator()( First& first, Args... tail )
    {
        write( first );
        (*this)( tail... );
    }
    
    void operator()() {}
    
    void write( bool value )
    {
#ifdef DEBUG
        assert( m_bufferPtr + 1 <= m_bufferEnd );
#endif
        *m_bufferPtr = value ? '\xFF' : '\x0';
        m_bufferPtr++;
    }
    
    void write( uint16_t number )
    {
#ifdef DEBUG
        assert( m_bufferPtr + 2 <= m_bufferEnd );
#endif
        *m_bufferPtr = number & 0x00FF;
        m_bufferPtr++;
        *m_bufferPtr = (number & 0xFF00) >> 8;
        m_bufferPtr++;
    }
    
    void write( const std::string& string )
    {
        write( static_cast<uint16_t>( string.size() ) );
        
#ifdef DEBUG
        assert( m_bufferPtr + string.size() <= m_bufferEnd );
#endif
        std::memcpy( m_bufferPtr, string.c_str(), string.size() );
        m_bufferPtr += string.size();
    }
    
    template<typename T>
    void write( const std::vector<T>& theVector )
    {
        write( static_cast<uint16_t>(theVector.size()) );
        
        for( const auto& element :  theVector )
        {
            write( element );
        }
    }
    
    template<typename T>
    void write( const T& tObject )
    {
        tObject.fields( *this );
    }
};

class PacketSize
{
    size_t m_size = 0;
    
public:
    PacketSize() {}
    
    size_t size() { return m_size; };
    
    template<typename First, typename ...Args>
    void operator()( First& first, Args... tail )
    {
        add_size( first );
        (*this)( tail... );
    }
    
    void operator()() {}
    
    void add_size( bool ) { m_size++; }
    void add_size( uint16_t ) { m_size+=2; }
    
    void add_size( const std::string& string )
    {
        m_size += (string.size() + 2);
    }
    
    template<typename T>
    void add_size( const std::vector<T>& theVector )
    {
        m_size += 2;
        
        for( const auto& element :  theVector )
        {
            add_size( element );
        }
    }
    
    template<typename T>
    void add_size( const T& tObject )
    {
        tObject.fields( *this );
    }
};

template<class PacketT>
uint8_t* createEnvelope( const std::string& playerName, PacketT& packet, size_t& outTcpPacketSize )
{
    PacketSize calculator;
    
    // TCP packet size
    calculator.add_size( uint16_t{} );
    
    // Player name
    calculator.add_size( playerName );

    // tic_tac packet type
    calculator.add_size( uint16_t{} );

    // packet bytes
    packet.fields( calculator );
    
    outTcpPacketSize = calculator.size();
    
    uint8_t* buffer = new uint8_t[outTcpPacketSize];
    PacketWriter writer( buffer, outTcpPacketSize );
    
    writer.write( static_cast<uint16_t>( outTcpPacketSize ) );
    writer.write( playerName );
    writer.write( (uint16_t) PacketT::packetType() );
    packet.fields( writer );
    
    return buffer;
}

}
