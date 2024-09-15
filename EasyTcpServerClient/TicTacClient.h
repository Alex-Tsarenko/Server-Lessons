#pragma once

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <map>
#include <string>

#include "TcpClient.h"
#include "TicTacProtocol.h"
#include "Logs.h"

namespace tic_tac {


class ITicTacClient
{
protected:
    virtual void onPlayerListChanged() = 0;

    virtual void onRegistered() = 0;

    // returned 'true'  -> if invitation accepted
    // returned 'false' -> if invitation rejected
    virtual void onInvitation( std::string playName ) = 0;
    
    // if 'isAccepted' and returned 'false' -> 'close game'
    // if 'isAccepted' and returned 'true' -> 'game started'
    // if '!isAccepted' and returned 'false' -> client go to state 'cst_waiting_user_choice'
    // if '!isAccepted' and returned 'true' -> client stays in state 'cst_inviting' (in case 2 or more player were invited by us)
    virtual void onAcceptedInvitation( std::string playName, bool isAccepted ) = 0;
    
    // returned 'false' -> client go to state 'cst_waiting_user_choice'
    // returned 'true' -> client stays in current state
    virtual void onPlayerOfflined( std::string playName ) = 0;

    virtual void onPartnerStep( bool isX, int x, int y ) = 0;
};

class TicTacClient: public TcpClient, ITicTacClient
{
    enum CurrentState {
        ttcst_initial,
        ttcst_handshaking,
        ttcst_registered,
    };
    
    CurrentState    m_currentState = ttcst_initial;
    std::string     m_partnerName;

    std::string     m_request;
    
protected:
    std::string                 m_playerName;
    std::map<std::string,bool>  m_availablePlayerList;
    
public:
    TicTacClient( std::string playerName ) : m_playerName(playerName) {}
    
protected:
    
    std::string clientName() const override { return m_playerName; }
    
    void sendInvitaion( std::string partnerName )
    {
        std::lock_guard<std::mutex>  lock(m_mutex);

        m_partnerName = partnerName;
        m_request = (CMT_INVITE) +  "," + partnerName;
        write( m_request );
    }

    void sendCloseGame( std::string partnerName )
    {
        std::lock_guard<std::mutex>  lock(m_mutex);

        m_partnerName = partnerName;
        m_request = (CMT_CLOSE_GAME) +  "," + partnerName;
        write( m_request );
    }

    void sendStep( std::string partnerName, std::string x_0, std::string x, std::string y )
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_partnerName = partnerName;
        m_request = (CMT_STEP) +  "," + partnerName + "," + x_0 + "," + x + "," + y;
        write( m_request );
    }

    void sendInvitaionResponse( std::string partnerName, bool isAccepted )
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if ( isAccepted )
        {
            m_partnerName = partnerName;
            m_request = (CMT_ACCEPT_INVITITAION) +  "," + partnerName;
            write( m_request );
        }
        else
        {
            m_request = (CMT_REJECT_INVITITAION) +  "," + partnerName;
            write( m_request );
            return;
        }
    }

    virtual void onMessageReceived( const std::string& message ) override
    {
        LOG( "> Client::onMessageReceived: (" << m_playerName.c_str() << "): " << message.c_str() );
        
        if ( message.empty() )
        {
            LOG_ERR( "Client::onMessageReceived empty response: '" << message.c_str() << "'" );
            return;
        }
        
        std::vector<std::string> tokens;
        boost::split( tokens, message, boost::is_any_of(",") );

        std::string messageType = tokens[0];
        
        if ( messageType == "Hi" )
        {
            if ( m_currentState != ttcst_initial )
            {
                LOG_ERR( "invalid server greeting: " << message.c_str() );
                return;
            }

            m_currentState = ttcst_handshaking;
            m_request = (CMT_PLAYER_NAME) + "," + m_playerName;
            write( m_request );
            return;
        }
        else if ( messageType == SMT_OK )
        {
            m_currentState = ttcst_registered;
            onRegistered();
            return;
        }
        else if ( messageType == SMT_PLAYER_LIST )
        {
            m_availablePlayerList.clear();
            for( size_t i=1; i<tokens.size(); i+=2 )
            {
                auto playerName = tokens[i];
                auto isAvailable = ! tokens[i+1].empty();
                LOG( "PlayerList[" << i-1 << "]: " << playerName.c_str() << " " << isAvailable );
                if ( playerName == m_playerName )
                {
                    continue;
                }
                
                m_availablePlayerList[playerName] = isAvailable;
            }

            onPlayerListChanged();
            return;
        }
        else if ( messageType == SMT_INVITITAION )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message.c_str() );
                return;
            }

            auto partnerName = tokens[1];

            onInvitation(partnerName);
            return;
        }
        else if ( messageType == SMT_PLAYER_OFFLINED )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message.c_str() );
                return;
            }

            auto playerName = tokens[1];
            onPlayerOfflined( playerName );
            return;
        }
        else if ( messageType == SMT_INVITITAION_REJECTED )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message.c_str() );
                return;
            }

            auto playerName = tokens[1];
            onAcceptedInvitation( playerName, false );
            return;
        }
        else if ( messageType == SMT_INVITITAION_ACCEPTED )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message.c_str() );
                return;
            }

            auto partnerName = tokens[1];
            onAcceptedInvitation( partnerName, true );
            return;
        }
        else if ( messageType == SMT_ON_STEP )
        {
            if ( tokens.size() < 5 )
            {
                LOG_ERR( "protocol error: tokens.size() " << message.c_str() );
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
        }
        else
        {
            LOG( "!!! unknown message type: " << messageType.c_str() );
#ifdef DEBUG
            assert(0);
#endif
        }
    }
};

}

