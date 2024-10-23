#include <boost/asio.hpp>
#include <iostream>
#include <ostream>
#include <strstream>
#include <map>
#include <optional>

#include <boost/algorithm/string.hpp>

#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TicTacPacketUtils.h"
#include "TcpServer.h"
#include "Logs.h"

namespace tic_tac {

class Session;

class Server
{
public:
    struct SessionInfo : public tic_tac::PlayerStatus
    {
        std::weak_ptr<Session> m_sessionPtr;
    };
    
    using PlayerNameString = std::string;
    std::map< PlayerNameString, SessionInfo > m_playerStatusMap;
    
public:
    Server() {}
    
    void onConnect( std::string playerName )
    {
        
        
    }
    
    void onDisconnect( std::string playerName )
    {
        
    }
    
    bool playerNameExists( const std::string& playerName ) const
    {
        return m_playerStatusMap.find( playerName ) != m_playerStatusMap.end();
    }

    void registerPlayer( const tic_tac::PlayerStatus& playerStatus, std::weak_ptr<Session> sessionPtr )
    {
        m_playerStatusMap[playerStatus.m_playerName] = SessionInfo{ playerStatus, sessionPtr };
    }

    void forgotPlayer( const std::string& playerName )
    {
        m_playerStatusMap.erase( playerName );
    }
    
    bool sendEnvelopTo( const std::string& playerTo, uint8_t* envelopBuffer, size_t envelopSize );
};



class Session : public std::enable_shared_from_this<Session>
{
    Server& m_server;
    std::string m_playerName;
    
public:
    Session( Server& server ) : m_server(server)
    {
    }
    
    void sendEnvelopFrom( uint8_t* envelop, size_t envelopSize )
    {
        static_cast< TcpClientSession<Server,Session>* >(this)->write( envelop, envelopSize );
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
    
        if ( ! playerName.empty() )
        {
            PacketSize sizeCalculator;
            sizeCalculator.add_size( playerName );
            auto offset = sizeCalculator.size();
            
            size_t   envelopSize;
            uint8_t* envelop = createEnvelope2( m_playerName, buffer, offset, envelopSize );
            
            m_server.sendEnvelopTo( playerName, envelop, envelopSize );
        }
        else
        {
            uint16_t type;
            reader.read( type );

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
                        
                        if ( m_server.playerNameExists( m_playerName ) )
                        {
                            ServerPacketPlayerAlreadyExists packet{};
                            sendEnvelopFrom( "", packet );
                            return false;
                        }
                        
                        std::vector<PlayerStatus> playerList;
                        playerList.reserve( m_server.m_playerStatusMap.size() );
                        std::transform( m_server.m_playerStatusMap.begin(), m_server.m_playerStatusMap.end(), std::back_inserter( playerList ), []( const auto& pair )
                        {
                            return static_cast<const PlayerStatus&>( pair.second );
                        });

                        ServerPacketPlayerList packet{ std::move(playerList) };
                        sendEnvelopFrom( "", packet );
                        
                        m_server.registerPlayer( PlayerStatus{ m_playerName, cst_accesible }, weak_from_this() );
                    }
                }
            }
        }
        return true;
    }
    
    void sendEnvelop( const uint8_t* bufferPtr, size_t bufferSize )
    {
        static_cast<TcpClientSession<Server,Session>*>(this) -> write( bufferPtr, bufferSize );
    }
};

inline bool Server::sendEnvelopTo( const std::string& playerTo, uint8_t* envelopBuffer, size_t envelopSize )
{
    auto it = m_playerStatusMap.find( playerTo );
    if ( it != m_playerStatusMap.end() )
    {
        if ( auto sessionPtr = it->second.m_sessionPtr.lock(); sessionPtr )
        {
            sessionPtr->sendEnvelop( envelopBuffer, envelopSize );
            return true;
        }
    }
    return false;
}


}
