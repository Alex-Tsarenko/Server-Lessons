#pragma once

#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TcpClient.h"

namespace tic_tac {

template<class UiClientT>
class Client: public UiClientT
{
    std::string m_playerName;
public:
    Client()  {}
    Client( std::string playerName ) : m_playerName(playerName) {}
    
    template<class Packet>
    void sendPacket( Packet )
    {
        
    }
    
    void onPacketReceived( const uint8_t* data, size_t dataSize )
    {
        //        switch( reinterpret_cast<const ServerPacketBase*>( data )->m_packetType )
        //        {
        //            case spt_undefined:
        //                LOG_ERR( "spt_undefined" );
        //                break;
        //            case spt_player_list:
        //                ServerPacketPlayerList packet;
        //                packet.parse( data, data + dataSize );
        //                break;
        //        }
    }
};

}
