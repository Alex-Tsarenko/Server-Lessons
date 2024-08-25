#pragma once

#include "TcpClient.h"
#include "TicTacProtocol.h"
#include "Logs.h"

namespace tic_tac {


class ITicTacClient
{
protected:
    virtual void onPlayerListChanged() = 0;
    virtual bool onInvitation( std::string playName ) = 0;
    virtual void onAcceptedInvitation( std::string playName, bool isNotRejected ) = 0;

    virtual void onPartnerStep( bool isX, int x, int y ) = 0;
};

class TicTacClient: public TcpClient, ITicTacClient
{
    enum CurrentState {
        cst_initial,
        cst_handshaking,
        cst_waiting_user_choice,
        cst_inviting,
        cst_gaming,
        cst_game_ending,
    };
    
    CurrentState    m_currentState = cst_initial;
    std::string     m_partnerName;

    std::string     m_request;
    
protected:
    std::string                 m_playerName;
    std::map<std::string,bool>  m_availablePlayList;
    
public:
    TicTacClient( std::string playerName ) : m_playerName(playerName) {}
    
protected:
    virtual void onMessageReceived( const std::string& message ) override
    {
        LOG( "Client::onMessageReceived: " << message );
        
        if ( message.empty() )
        {
            LOG_ERR( "Client::onMessageReceived empty response: '" << message << "'" );
            return;
        }
        
        std::vector<std::string> tokens;
        boost::split( tokens, message, boost::is_any_of(",") );

        std::string command = tokens[0];
        
        if ( command == "Hi" )
        {
            if ( m_currentState != cst_initial )
            {
                LOG_ERR( "invalid server greeting: " << message );
                return;
            }

            m_currentState = cst_handshaking;
            m_request = "Hello," + m_playerName;
            write( m_request );
            return;
        }
        else if ( command == CMD_OK )
        {
            if ( m_currentState != cst_handshaking )
            {
                LOG_ERR( "invalid server response: " << message );
                return;
            }

            m_currentState = cst_waiting_user_choice;
            return;
        }
        else if ( command == CMD_PLAYER_LIST )
        {
            if ( m_currentState == cst_initial || m_currentState == cst_handshaking )
            {
                LOG_ERR( "protocol error: (1)" << message );
                return;
            }

            m_availablePlayList.clear();
            for( size_t i=1; i<tokens.size(); i+=2 )
            {
                auto playerName = tokens[i];
                auto isAvailable = ! tokens[i+1].empty();
                LOG( "PlayerList[" << i-1 << "]: " << playerName << " " << isAvailable );
                if ( playerName == m_playerName )
                {
                    continue;
                }
                
                m_availablePlayList[playerName] = isAvailable;
            }

            onPlayerListChanged();
            return;
        }
        else if ( command == CMD_INVITITAION )
        {
            if ( m_currentState == cst_initial || m_currentState == cst_handshaking )
            {
                LOG_ERR( "protocol error: (2)" << message );
                return;
            }
            
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message );
                return;
            }

            auto partnerName = tokens[1];

            if ( m_currentState != cst_waiting_user_choice  || ! onInvitation(partnerName) )
            {
                m_request = std::string(CMD_REJECT_INVITITAION) +  "," + partnerName;
                write( m_request );
                return;
            }
            
            m_partnerName = partnerName;
            m_request = std::string(CMD_ACCEPT_INVITITAION) +  "," + partnerName;
            write( m_request );
            m_currentState = cst_gaming;
            return;
        }
        else if ( command == CMD_INVITITAION_REJECTED )
        {
            if ( m_currentState != cst_inviting )
            {
                LOG_ERR( "protocol error: (3)" << message );
                return;
            }
            auto playerName = tokens[1];
            onAcceptedInvitation( playerName, false );
            m_currentState = cst_waiting_user_choice;
            return;
        }
        else if ( command == CMD_INVITITAION_ACCEPTED )
        {
            if ( m_currentState != cst_inviting )
            {
                LOG_ERR( "protocol error: (4)" << message );

                if ( tokens.size() < 2 )
                {
                    LOG_ERR( "protocol error: tokens.size() " << message );
                    return;
                }

                auto playerName = tokens[1];
                m_request = std::string(CMD_CLOSE_GAME) + "," + playerName;
                write( m_request );
                return;
            }
            m_currentState = cst_gaming;

//            auto partnerName = tokens[1];
//            std::string x_or_0;
//            int x;
//            int y;
//            myStep( x_or_0, x, y );
//            m_request = CMD_STEP + "," + partnerName + "," + x_or_0 + "," + std::to_string(x) + "," + std::to_string(y);
//            write( m_request );
            return;
        }
        else if ( command == CMD_STEP )
        {
            if ( tokens.size() < 5 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message );
                return;
            }

            if ( m_currentState != cst_gaming )
            {
                LOG_ERR( "protocol error: (5)" << message );

                auto playerName = tokens[1];
                m_request = std::string(CMD_CLOSE_GAME) + "," + playerName;
                write( m_request );
                return;
            }
            
            auto partnerName = tokens[1];
            auto x_or_0 = tokens[2];
            int x = std::stoi(tokens[3]);
            int y = std::stoi(tokens[4]);
            onPartnerStep( x_or_0 != "0", x, y );
        }
        else if ( command == CMD_GAME_ENDED )
        {
            
        }
        else
        {
            //todo log error
        }
    }
};

}

