#pragma once

#include "TicTacClient.h"
#include "TicTacProtocol.h"

namespace tic_tac {

class DbgTicTacClient : public tic_tac::TicTacClient
{
public:
    DbgTicTacClient( std::string palyerName ) : TicTacClient(palyerName) {}
    
protected:
    void onPlayerListChanged() override
    {
        for( auto [playerName,isAvailable] : m_availablePlayerList )
        {
            if ( playerName != m_playerName && isAvailable )
            {
                std::thread([playerName=playerName,this] { sendInvitaion(playerName); } ).detach();
                return;
            }
        }
    }
    
    void onInvitation( std::string partnerName ) override
    {
        sleep(1);
        sendInvitaionResponse( partnerName, true );
        usleep(1000);
        sendStep(partnerName, "X","1","1" );
    }
    
    void onAcceptedInvitation( std::string playName, bool isAccepted ) override
    {
    }
    
    virtual void onPlayerOfflined( std::string playName ) override
    {
    }
    
    void onPartnerStep( bool isX, int x, int y ) override
    {
    }

    void onRegistered() override
    {
    }
};

} // namespace tic_tac {
