#pragma once

#include "TicTacClient.h"

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
                std::string message = "Invitation,"+m_playerName;
                write(message);
                return;
            }
        }
    }
    bool onInvitation( std::string playName ) override
    {
        return true;
//        std::string message = "AcceptInvitation";
//        write(message);
//        message = "Step,X,1,1";
//        write(message);
    }
    
    bool onAcceptedInvitation( std::string playName, bool isNotRejected ) override
    {
        //todo
    }

    virtual bool onPlayerOfflined( std::string playName ) override
    {
        //todo
    }
    
    void onPartnerStep( bool isX, int x, int y ) override
    {
        
    }
};

