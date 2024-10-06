#pragma once

#include <iostream>
#include <vector>

enum ServerPacketType : uint16_t {
    spt_undefined = 200,
    spt_player_list,
};

struct ServerPacketBase
{
    constexpr ServerPacketType packetType() const { return spt_undefined; }
};

struct ServerPacketPlayerList: public ServerPacketBase
{
    std::vector<std::string> m_playerList;
    
    ServerPacketPlayerList() {}
    
    constexpr ServerPacketType packetType() const { return spt_player_list; }

    void parse( const uint8_t* (&buffer), const uint8_t* bufferEnd )
    {
        //TODO
    }
    
    uint16_t calculateDataSize()
    {
        //return static_cast<uint16_t>( sizeof(m_packetType) + sizeof(uint16_t) + m_playerName.size() );
    }
    
    void writeToBuffer( uint8_t* buffer )
    {
        //TODO
    }
};
