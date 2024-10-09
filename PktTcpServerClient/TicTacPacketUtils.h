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
    
private:
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
    
    void read( PlayerStatus& playerStatus )
    {
        read( playerStatus.m_playerName );
        read( reinterpret_cast<uint16_t&>(playerStatus.m_status) );
    }
};

class PacketWriter
{
    uint8_t* m_bufferPtr;
    uint8_t* m_bufferEnd;
    
public:
    PacketWriter( uint8_t*     bufferPtr,
                 size_t    tcpPacketSize,
                 uint16_t  packetType )
    :
    m_bufferPtr( bufferPtr ),
    m_bufferEnd( bufferPtr+tcpPacketSize )
    {
#ifdef DEBUG
        assert( tcpPacketSize <= 0xFFFF );
#endif
        // write TCP packet size
        write( static_cast<uint16_t>( tcpPacketSize ) );
        
        // write applied packet typee
        write( packetType );
    }
    
    template<typename First, typename ...Args>
    void operator()( First& first, Args... tail )
    {
        write( first );
        (*this)( tail... );
    }
    
    void operator()() {}
    
private:
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
            read( element );
        }
    }
    
    void write( const PlayerStatus& playerStatus )
    {
        write( playerStatus.m_playerName );
        write( static_cast<uint16_t>(playerStatus.m_status) );
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
        size( first );
        (*this)( tail... );
    }
    
    void operator()() {}
    
private:
    void size( bool ) { m_size++; }
    void size( uint16_t ) { m_size+=2; }
    
    void size( const std::string& string )
    {
        m_size += (string.size() + 2);
    }
    
    template<typename T>
    void size( const std::vector<T>& theVector )
    {
        m_size += 2;
        
        for( const auto& element :  theVector )
        {
            size( element );
        }
    }
    
    void size( const PlayerStatus& playerStatus )
    {
        size( playerStatus.m_playerName );
        size( static_cast<uint16_t>(playerStatus.m_status) );
    }
    
};

template<class PacketT>
PacketT readPacket( const uint8_t* buffer, size_t bufferSize )
{
    PacketT packet;
    PacketReader reader( buffer, buffer+ bufferSize );
    packet.fields( reader );
    return packet;
}

template<class PacketT>
void writePacket( PacketT& packet, uint8_t* buffer, size_t bufferSize )
{
    PacketWriter writer( buffer, bufferSize, PacketT::packetType() );
    packet.fields( writer );
}

template<class PacketT>
uint8_t* createOutgoingMessage( PacketT& packet, size_t& outRcpPacketSize )
{
    PacketSize calculator;
    packet.fields( calculator );
    outRcpPacketSize = calculator.size() + 4;
    
    uint8_t* buffer = new uint8_t[outRcpPacketSize];
    PacketWriter writer( buffer, outRcpPacketSize, packet.packetType() );
    packet.fields( writer );
    
    return buffer;
}

}
