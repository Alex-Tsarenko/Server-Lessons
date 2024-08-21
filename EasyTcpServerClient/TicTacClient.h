#pragma once

#include "TcpClient.h"
#include "Logs.h"

class ITicTacClient
{
protected:
    virtual void onPlayerListChanged() = 0;
    virtual void onInvitation( std::string playName ) = 0;
    virtual void onAcceptedInvitation( std::string playName, bool isNotRejected ) = 0;

    virtual void onPartnerStep( bool isX, int x, int y ) = 0;
};

class TicTacClient: public TcpClient, ITicTacClient
{
    enum CurrentState { cst_initial, cst_handshaking, cst_waiting_user_choice, cst_suggesting_game, cst_gaming };
    
    CurrentState    m_currentState = cst_initial;
    
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

        switch( m_currentState )
        {
            case cst_initial:
            {
                if ( message != "Hi" )
                {
                    LOG_ERR( "invalid server greeting: " << message );
                    return;
                }
                m_currentState = cst_handshaking;
                m_request = "Hello," + m_playerName;
                write( m_request );
                break;
            }
            case cst_handshaking:
            {
                if ( message != "Ok" )
                {
                    LOG_ERR( "invalid server 'Ok' response: " << message );
                    return;
                }
                m_currentState = cst_waiting_user_choice;
                break;
            }
            //
            // cst_waiting_user_choice:
            //
            case cst_waiting_user_choice:
            {
                std::vector<std::string> tokens;
                boost::split( tokens, message, boost::is_any_of(",") );

                auto& messageType = tokens[0];
                
                if ( messageType != "PlayerList" && tokens[0] != "InvitaionFrom" )
                {
                    LOG_ERR( "invalid server 'PlayerList' response: " << message );
                    return;
                }
                else if ( tokens[0] == "InvitaionFrom" )
                {
                    auto playerName = tokens[1];
                    onInvitation( playerName );
                }
                else if ( tokens[0] == "PlayerList" )
                {
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
                }
                else if ( tokens[0] == "AcceptedFrom" )
                {
                    auto playerName = tokens[1];
                    bool isNotRejected = tokens[2] == "1";
                    onAcceptedInvitation( playerName, isNotRejected );
                }
                else if ( tokens[0] == "Step" )
                {
                    auto isX = (tokens[1] == "X");
                    int x = std::stoi(tokens[2]);
                    int y = std::stoi(tokens[3]);
                    onPartnerStep( isX, x, y );
                }
                break;
            }
        }
    }
};

