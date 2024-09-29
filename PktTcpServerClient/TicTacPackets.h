#pragma once

#include <iostream>
#include <vector>

struct PacketBase
{
    //std::vector<char> m_outputBuffer;
    
    void writeUint16( char* (&buffer), uint16_t number )
    {
        buffer[0] = number & 0x00FF;
        buffer[1] = (number & 0xFF00) >> 8;
        buffer += 2;
    }
    void writeString( char* (&buffer), const std::string& string )
    {
        writeUint16( buffer, static_cast<uint16_t>( string.size() ) );
        std::memcpy( buffer, string.c_str(), string.size() );
        buffer += string.size();
    }
    
    uint16_t readUint16( const char* (&buffer) )
    {
        int number = buffer[0] | (buffer[1] << 8);
        buffer += 2;
        return static_cast<uint16_t>( number );
    }
    
    std::string readString( const char* (&buffer), const char* bufferEnd )
    {
        std::string outString;
        
        auto stringLenght = readUint16( buffer );
        if ( buffer + stringLenght >= buffer )
        {
            throw std::runtime_error("Invalid string length: " + std::to_string(stringLenght) );
        }
        
        outString.resize( stringLenght );
        std::memcpy( const_cast<char*>(outString.c_str()), buffer, stringLenght );
        buffer += stringLenght;
    }
};

enum ClientPacketType : uint16_t {
    cpt_undefined = 100,
    cpt_hi,
    cpt_invite,
};

struct ClientPacketBase: public PacketBase
{
    ClientPacketType m_packetType = cpt_undefined;
};

struct ClientPacketHi: public ClientPacketBase
{
    std::string m_playerName;
    
    ClientPacketHi() {}
    ClientPacketHi( std::string playerName ) : m_playerName(playerName) {}
    
    void parse( const char* (&buffer), const char* bufferEnd )
    {
        m_playerName = readString( buffer, bufferEnd );
    }
    
    uint16_t calculateDataSize()
    {
        return static_cast<uint16_t>( sizeof(m_packetType) + sizeof(uint16_t) + m_playerName.size() );
    }
    
    void writeToBuffer( char* buffer )
    {
        writeUint16( buffer, m_packetType );
        writeString( buffer, m_playerName );
    }
};

enum ServerPacketType : uint16_t {
    spt_undefined = 200,
    spt_player_list,
};

struct ServerPacketBase: public PacketBase
{
    ServerPacketType m_packetType = spt_undefined;
};

struct ServerPacketPlayerList: public ServerPacketBase
{
    std::vector<std::string> m_playerList;
    
    ServerPacketPlayerList() {}
    
    void parse( const char* (&buffer), const char* bufferEnd )
    {
        //TODO
    }
    
    uint16_t calculateDataSize()
    {
        //return static_cast<uint16_t>( sizeof(m_packetType) + sizeof(uint16_t) + m_playerName.size() );
    }
    
    void writeToBuffer( char* buffer )
    {
        //TODO
    }
};
