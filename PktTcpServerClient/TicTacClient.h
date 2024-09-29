#pragma once

#include "TicTacPackets.h"
#include "TcpClient.h"

class TicTacClientBase
{
    std::string m_playerName;
public:
    TicTacClientBase()  {}
    TicTacClientBase( std::string playerName ) : m_playerName(playerName) {}

    void onPacketReceived( const char* data, size_t dataSize )
    {
        switch( reinterpret_cast<const ServerPacketBase*>( data )->m_packetType )
        {
            case spt_undefined:
                LOG_ERR( "spt_undefined" );
                break;
            case spt_player_list:
                ServerPacketPlayerList packet;
                packet.parse( data, data + dataSize );
                break;
        }
    }
};
