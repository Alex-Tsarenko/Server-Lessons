#pragma once

#import "TicTacPacketUtils.h"
#import "TicTacClientPackets.h"

class DbgUiClient
{
protected:
    std::string m_playerName;
    
public:
    
    DbgUiClient( std::string playerName ) : m_playerName(playerName) {}
    
    void onPlayerListReceived( const std::vector<tic_tac::PlayerStatus> playerList )
    {
        std::optional<tic_tac::PlayerStatus> availablePlayer;
        
        for( const auto& info: playerList )
        {
            if ( info.m_status == tic_tac::cst_accesible )
            {
                LOG( "PlayerList: " << m_playerName << " " << info.m_playerName );
                availablePlayer = info;
            }
        }
        
        if ( availablePlayer )
        {
            LOG( "Send Invititaion: " << m_playerName << " to: " << availablePlayer->m_playerName );
            sendPacketTo( availablePlayer->m_playerName, tic_tac::PacketInvite{} );
        }
        
    }
    
    void onInviteReceivedFrom( const std::string& partnerName )
    {
        LOG( "onInviteReceivedFrom: " << m_playerName << ": from:  " << partnerName );
    }

    
    template<class PacketT>
    void sendPacketTo( const std::string toPlayerName, const PacketT& packet )
    {
        size_t   bufferSize;
        uint8_t* buffer = createEnvelope( toPlayerName, packet, bufferSize );
        
        static_cast<TcpClient<tic_tac::Client<DbgUiClient>>*>(this)->write( buffer, bufferSize );
    }
};
