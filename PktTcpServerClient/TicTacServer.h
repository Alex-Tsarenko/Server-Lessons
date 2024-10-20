#include <boost/asio.hpp>
#include <iostream>
#include <ostream>
#include <strstream>
#include <optional>

#include <boost/algorithm/string.hpp>

#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TicTacPacketUtils.h"
#include "TcpServer.h"
#include "Logs.h"

namespace tic_tac {

class Server
{
public:
    std::vector<tic_tac::PlayerStatus> m_playerList;
    
public:
    Server() {}
    
    void onConnect( std::string playerName )
    {
        
        
    }
    
    void onDisconnect( std::string playerName )
    {
        
    }
};



class Session
{
    Server& m_server;
    std::string m_playerName;
    
public:
    Session( Server& server ) : m_server(server)
    {
    }
    
    template<class PacketT>
    void sendEnvelopFrom( std::string playerFrom, PacketT& packet )
    {
        size_t envelopSize;
        uint8_t* envelop = createEnvelope( "", packet, envelopSize );
        static_cast< TcpClientSession<Server,Session>* >(this)->write( envelop, envelopSize );
    }

    bool onPacketReceived( const std::vector<uint8_t>& buffer )
    {
        tic_tac::PacketReader reader( buffer.data(), buffer.data()+buffer.size() );
        
        std::string playerName;
        reader.read( playerName );
    
        uint16_t type;
        reader.read( type );

        if ( playerName.empty() )
        {
            switch( type )
            {
                case cpt_hi:
                {
                    if ( m_playerName.empty() )
                    {
                        reader.read( m_playerName );
                        if ( m_playerName.empty() )
                        {
                            return false;
                        }
                        
                        ServerPacketPlayerList packet{ m_server.m_playerList };
                        sendEnvelopFrom( "", packet );
                    }
                }
            }
        }
        else
        {
            PacketType packetType;
            reader.read( reinterpret_cast<uint16_t&>(packetType) );
            
            switch( packetType )
            {
                case tic_tac::cpt_hi:
                {
                    tic_tac::PacketHi packet("");
                    packet.fields( reader);
                    LOG("");
                    
                    //todo
                    PacketClientStatus status{ playerName, cst_not_accesible };
                    sendPacket( status, playerName  );
                    break;
                }
                default:
            }
        }
        return true;
    }
    
    template<class PacketT>
    void sendPacket( PacketT& packet, const std::string& playerName  )
    {
        //todo envelop
        
        PacketSize packetSizeCalculator;
        packet.fields( packetSizeCalculator );
        size_t packetSize = packetSizeCalculator.size();
    
        //todo packetSize == 0
        if ( packetSize > 0 )
        {
            uint8_t* buffer = new uint8_t[packetSize];
            PacketWriter writer( buffer, packetSize);
            sendPacket( buffer, packetSize );
        }
    }
    
    void sendPacket( const uint8_t* bufferPtr, size_t bufferSize )
    {
        static_cast<TcpClientSession<Server,Session>*>(this) -> write( bufferPtr, bufferSize );
    }
    
};

}
