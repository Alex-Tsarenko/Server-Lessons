#pragma once

#include "TcpClient.h"
#include "Logs.h"

class Client: public TcpClient
{
    enum CurrentState { cst_initial, cst_handshaking, cst_waiting_list,  };
    
    CurrentState    m_currentState = cst_initial;
    
    std::string     m_playerName;
    
    std::string     m_request;
    
public:
    Client( std::string playerName ) : m_playerName(playerName) {}
    
protected:
    virtual void onMessageReceived( const std::string& message ) override
    {
        LOG( "Client::onMessageReceived: " << message );
        
        if ( message.empty() )
        {
            std::cerr << "Client::onMessageReceived empty response: '" << message << "'" << std::endl;
            return;
        }

//        if ( message.back() != '\0' )
//        {
//            std::cerr << "Client::onMessageReceived invalid response: '" << message.back() << "'" << std::endl;
//            return;
//        }

        switch( m_currentState )
        {
            case cst_initial:
            {
                if ( message != "Hi" )
                {
                    std::cerr << "invalid server greeting: " << message << std::endl;
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
                    std::cerr << "invalid server 'Ok' response: " << message << std::endl;
                    return;
                }
                m_currentState = cst_waiting_list;
                write( "GetPlayers," );
                break;
            }
            case cst_waiting_list:
            {
                std::vector<std::string> tokens;
                boost::split( tokens, message, boost::is_any_of(",") );

                if ( tokens[0] != "PlayerList" )
                {
                    std::cerr << "invalid server 'PlayerList' response: " << message << std::endl;
                    return;
                }
                
                for( size_t i=1; i<tokens.size(); i++ )
                {
                    LOG( "PlayerList[" << i-1 << "]: " << tokens[i] );
                }
                break;
            }
        }
    }
};

