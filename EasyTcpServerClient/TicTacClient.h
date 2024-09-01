#pragma once

#include "TcpClient.h"
#include "TicTacProtocol.h"
#include "Logs.h"

namespace tic_tac {


class ITicTacClient
{
protected:
    virtual void onPlayerListChanged() = 0;
    
    // returned 'true'  -> if invitation accepted
    // returned 'false' -> if invitation rejected
    virtual bool onInvitation( std::string playName ) = 0;
    
    // if 'isAccepted' and returned 'false' -> 'close game'
    // if 'isAccepted' and returned 'true' -> 'game started'
    // if '!isAccepted' and returned 'false' -> client go to state 'cst_waiting_user_choice'
    // if '!isAccepted' and returned 'true' -> client stays in state 'cst_inviting' (in case 2 or more player were invited by us)
    virtual bool onAcceptedInvitation( std::string playName, bool isAccepted ) = 0;
    
    // returned 'false' -> client go to state 'cst_waiting_user_choice'
    // returned 'true' -> client stays in current state
    virtual bool onPlayerOfflined( std::string playName ) = 0;

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
    
    std::string playerName() const override { return m_playerName; }
    
    void sendInvitaion( std::string partnerName)
    {
        std::lock_guard<std::mutex>  lock(m_mutex);
        
        m_partnerName = partnerName;
        m_request = (CMT_INVITE) +  "," + partnerName;
        write( m_request );
        
        m_currentState = cst_inviting;
    }
    
    void sendStep( std::string partnerName, std::string x_0, std::string x, std::string y )
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_partnerName = partnerName;
        m_request = (CMT_STEP) +  "," + partnerName + "," + x_0 + "," + x + "," + y;
        write( m_request );
    }
    
    virtual void onMessageReceived( const std::string& message ) override
    {
        LOG( "> Client::onMessageReceived: (" << m_playerName << "): " << message );
        
        if ( message.empty() )
        {
            LOG_ERR( "Client::onMessageReceived empty response: '" << message << "'" );
            return;
        }
        
        std::vector<std::string> tokens;
        boost::split( tokens, message, boost::is_any_of(",") );

        std::string messageType = tokens[0];
        
        if ( messageType == "Hi" )
        {
            if ( m_currentState != cst_initial )
            {
                LOG_ERR( "invalid server greeting: " << message );
                return;
            }

            m_currentState = cst_handshaking;
            m_request = (CMT_PLAYER_NAME) + "," + m_playerName;
            write( m_request );
            return;
        }
        else if ( messageType == SMT_OK )
        {
            if ( m_currentState != cst_handshaking )
            {
                LOG_ERR( "invalid server response: " << message );
                return;
            }

            m_currentState = cst_waiting_user_choice;
            return;
        }
        else if ( messageType == SMT_PLAYER_LIST )
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
        else if ( messageType == SMT_INVITITAION )
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
                m_request = (CMT_REJECT_INVITITAION) +  "," + partnerName;
                write( m_request );
                return;
            }
            
            m_partnerName = partnerName;
            m_request = (CMT_ACCEPT_INVITITAION) +  "," + partnerName;
            write( m_request );
            m_currentState = cst_gaming;
            return;
        }
        else if ( messageType == SMT_PLAYER_OFFLINED )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message );
                return;
            }

            auto playerName = tokens[1];
            if ( onPlayerOfflined( playerName ) )
            {
                m_currentState = cst_waiting_user_choice;
            }
            return;
        }
        else if ( messageType == SMT_INVITITAION_REJECTED )
        {
            if ( m_currentState != cst_inviting )
            {
                LOG_ERR( "protocol error: (3)" << message );
                return;
            }

            if ( tokens.size() < 2 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message );
                return;
            }

            auto playerName = tokens[1];
            if ( ! onAcceptedInvitation( playerName, false ) )
            {
                m_currentState = cst_waiting_user_choice;
            }
            return;
        }
        else if ( messageType == SMT_INVITITAION_ACCEPTED )
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
                m_request = (CMT_CLOSE_GAME) + "," + playerName;
                write( m_request );
                return;
            }
            m_currentState = cst_gaming;

            auto partnerName = tokens[1];
            if ( ! onAcceptedInvitation( partnerName, true ) )
            {
                m_request = (CMT_CLOSE_GAME) + "," + partnerName;
                write( m_request );
                return;
            }
            m_currentState = cst_gaming;
            return;
        }
        else if ( messageType == SMT_ON_STEP )
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
                m_request = (CMT_CLOSE_GAME) + "," + playerName;
                write( m_request );
                return;
            }
            
            auto partnerName = tokens[1];
            auto x_or_0 = tokens[2];
            int x = std::stoi(tokens[3]);
            int y = std::stoi(tokens[4]);
            onPartnerStep( x_or_0 != "0", x, y );
        }
        else if ( messageType == SMT_GAME_IS_OVER )
        {
            m_currentState = cst_waiting_user_choice;
        }
        else
        {
            LOG( "!!! unknown message type: " << messageType );
#ifdef DEBUG
            assert(0);
#endif
        }
    }
};

}

