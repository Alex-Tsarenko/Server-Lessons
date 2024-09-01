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
        for( auto [playerName,isAvailable] : m_availablePlayList )
        {
            if ( playerName != m_playerName && isAvailable )
            {
                std::thread([playerName=playerName,this] { sendInvitaion(playerName); } ).detach();
                return;
            }
        }
    }
    
    bool onInvitation( std::string playerName ) override
    {
        std::thread([playerName=playerName,this] { sendStep(playerName, "X","1","1" ); } ).detach();
        
        // accept invitation
        return true;
    }
    
    bool onAcceptedInvitation( std::string playName, bool isAccepted ) override
    {
        return true;
    }
    
    virtual bool onPlayerOfflined( std::string playName ) override
    {
        //todo
    }
    
    void onPartnerStep( bool isX, int x, int y ) override
    {
        return;
    }
};

} // namespace tic_tac {
