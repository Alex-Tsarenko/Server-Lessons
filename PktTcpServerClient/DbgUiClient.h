#pragma once

#import "TicTacPacketUtils.h"

class DbgUiClient
{
    std::string m_playerName;
public:
    
    virtual void doWrite( const uint8_t* message, size_t size ) = 0;
    void initClient( const std::string& playerName )
    {
        m_playerName = playerName;
        
        tic_tac::ClientPacketHi hiPacket( playerName );
        sendPacket( hiPacket );
        
    }
    
    template<class PacketT>
    void sendPacket( PacketT& packet )
    {
        size_t bufferSize;
        const uint8_t* buffer = createOutgoingMessage( packet, bufferSize );
        doWrite( buffer, bufferSize );
    }
};
