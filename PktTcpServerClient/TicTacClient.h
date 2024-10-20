#pragma once

#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TicTacPacketUtils.h"
#include "TcpClient.h"

namespace tic_tac {

template<class UiClientT>
class Client: public UiClientT
{
public:
    Client( std::string playerName ) : UiClientT(playerName) {}
    
    template<class Packet>
    void sendPacketTo( Packet& packet, const std::string playerName )
    {
        size_t bufferSize;
        uint8_t* buffer = createEnvelope( "", packet, bufferSize );
        static_cast<TcpClient<tic_tac::Client<UiClientT>>*>(this)->write( buffer, bufferSize );
    }
    
    void onConnect( const boost::system::error_code& ec )
    {
        if (!ec) {
            LOG( "Successfully connected to the server!" );
            PacketHi packet{ UiClientT::m_playerName };
            sendPacketTo( packet, {} );
        } else {
            std::cerr << "Error connecting: " << ec.message() << "\n";
        }
    }

    void onPacketReceived( const uint8_t* data, size_t dataSize )
    {
        tic_tac::PacketReader reader( data, data+dataSize );
        
        std::string playerName;
        reader.read( playerName );
        if ( playerName.empty() )
        {
            PacketType packetType;
            reader.read( reinterpret_cast<uint16_t&>(packetType) );
            
            switch( packetType )
            {
                case spt_player_list:
                {
                    ServerPacketPlayerList packet;
                    reader.read( packet );
                    UiClientT::onPlayerListReceived( packet.m_playerList );
                    break;
                }
            }
        }
        else
        {
            // Packets from another player
            PacketType packetType;
            reader.read( reinterpret_cast<uint16_t&>(packetType) );
            
            switch( packetType )
            {
                case cpt_invite:
                    PacketInvite packet;
                    reader.read( packet );
                    UiClientT::onInviteReceivedFrom( playerName );
                    break;
            }
        }
    }
};

}
