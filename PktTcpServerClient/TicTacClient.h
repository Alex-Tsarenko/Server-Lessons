#pragma once

#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TicTacPacketUtils.h"
#include "TcpClient.h"

namespace tic_tac {

template<class UiClientT>
class Client: public UiClientT
{
    std::string m_playerName;
public:
    Client()  {}
    Client( std::string playerName ) : m_playerName(playerName) {}
    
    void initClient( const std::string& playerName )
    {
        m_playerName = playerName;
    }

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
            std::cout << "Successfully connected to the server!\n";
            PacketHi packet{ m_playerName };
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
                    break;
                }
            }
        }
        else
        {
            PacketType packetType;
            reader.read( reinterpret_cast<uint16_t&>(packetType) );
            
            switch( packetType )
            {
            }
        }
    }
};

}
