#include "TcpServer.h"
#include "TicTacProtocol.h"
#include "Logs.h"
#include <map>

namespace tic_tac {

//class  TicTacServer;
struct TicTacClientSession;

// TicTacServer - abstart class (interface) that defines 'addClient' abstract function
//
// because we use 1 header for 2 classes and 1-st class cannot refers to members of 2-d class
//
class ITicTacServer
{
public:
    using ClientName = const std::string;
    
    virtual bool addClient( ClientName&, const std::weak_ptr<TicTacClientSession>&, std::string& errorText ) = 0;
    virtual std::string getPlayerListResponse() = 0;
    
    virtual void sendPlayerListToAll() = 0;
    
    virtual bool sendInvitaion( std::string senderPlayerName, std::string playerName, std::string& outErrorText ) = 0;
    
    virtual bool sendInvitaionAccepted( bool isAccepted, std::string senderPlayerName, std::string playerName, std::string& outErrorText ) = 0;
    
    virtual bool sendStep( std::string playerName, std::string x_0, std::string x, std::string y ) = 0;
    
    virtual bool sendCloseGame( std::string playerName, std::string otherPlayerName ) = 0;
};


// Session derived from TCP session
// It is from applied/subject level of the program
// It related to subject logic
//
class TicTacClientSession: public TcpClientSession
{
    ITicTacServer& m_ticTacServer;
    std::string    m_playerName;
    
    std::weak_ptr<TicTacClientSession>  m_otherPlayer;
    
public:
    TicTacClientSession( ITicTacServer& ticTacServer, boost::asio::ip::tcp::socket&& socket )
    :
    TcpClientSession( std::move(socket) ), m_ticTacServer(ticTacServer)
    {}
    
    ~TicTacClientSession()
    {
        LOG( "~TicTacClientSession: " << this );
    }
    
    void onMessage( const std::string& request ) override
    {
        LOG( "TicTacClientSession::onMessage: " << request );
        
        if ( request.empty() )
        {
            LOG_ERR( "TcpClientSession bad request: " << request );
            return;
        }
        
        std::vector<std::string> tokens;
        boost::split( tokens, request, boost::is_any_of(",") );
        
        std::string messageType = tokens[0];
        
        if ( messageType == CMT_PLAYER_NAME )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "TcpClientSession bad request (2): " << request );
                return;
            }
            
            m_playerName = tokens[1];
            LOG( "playerName: " << m_playerName );
            
            std::string errorText;
            if ( ! m_ticTacServer.addClient( m_playerName, std::dynamic_pointer_cast<TicTacClientSession>( shared_from_this() ), errorText ) )
            {
                std::string response = (SMT_ON_ERROR) + "," + errorText + ";";
                write( response );
                return;
            }
            
            std::string response = (SMT_OK) + ";";
            write( response );
            
            m_ticTacServer.sendPlayerListToAll();
        }
        else if ( messageType == CMT_INVITE )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "TcpClientSession bad request (3): " << request );
                return;
            }
            
            auto& otherPlayerName = tokens[1];
            LOG( "otherPlayerName: " << otherPlayerName );
            
            std::string outErrorText;
            
            if ( ! m_ticTacServer.sendInvitaion( m_playerName, otherPlayerName, outErrorText ) )
            {
                LOG( "Invite error: " << outErrorText );
                std::string response = (SMT_INVITITAION_REJECTED) + "," + otherPlayerName + ";";
                write( response );
            }
        }
        else if ( messageType == CMT_ACCEPT_INVITITAION || messageType == CMT_REJECT_INVITITAION )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "TcpClientSession bad request (4): " << request );
                return;
            }
            
            auto& otherPlayerName = tokens[1];
            LOG( "otherPlayerName: " << otherPlayerName );
            
            std::string outErrorText;
            
            if ( ! m_ticTacServer.sendInvitaionAccepted( (messageType == CMT_ACCEPT_INVITITAION), m_playerName, otherPlayerName, outErrorText ) )
            {
                LOG( "Invite error: " << outErrorText );
                std::string response = (SMT_PLAYER_OFFLINED) + "," + otherPlayerName + ";";
                write( response );
            }
        }
        else if ( messageType == CMT_STEP )
        {
            if ( tokens.size() != 5 )
            {
                LOG_ERR( "TcpClientSession bad request (4): " << request );
                return;
            }

            if ( ! m_ticTacServer.sendStep( tokens[1], tokens[2], tokens[3], tokens[4] ) )
            {
                std::string response = (SMT_PLAYER_OFFLINED) + "," + tokens[1] + ";";
                write( response );
            }
        }
        else if ( messageType == CMT_CLOSE_GAME )
        {
            if ( tokens.size() < 2 )
            {
                LOG_ERR( "TcpClientSession bad request (4): " << request );
                return;
            }
            
            auto& otherPlayerName = tokens[1];
            LOG( "otherPlayerName: " << otherPlayerName );
            
            std::string outErrorText;
            
            if ( m_ticTacServer.sendCloseGame( m_playerName, otherPlayerName ) )
            {
                LOG( "Invite error: " << outErrorText );
                std::string response = (SMT_PLAYER_OFFLINED) + "," + otherPlayerName + ";";
                write( response );
            }
        }
        else if ( messageType == "PlayerStep" )
        {
        }
        else
        {
            LOG_ERR( "Unnkown message type: " << messageType );
        }
        
        read();
    }
};

// Server - derived from TCP server
// It provides 2 methods:
//    createSession() for base class
//    addClient() for session
//
// Plus it contains map of sessioons
//
class TicTacServer: public TcpServer, public ITicTacServer
{
    std::map<ClientName,std::weak_ptr<TicTacClientSession>> m_clientMap;
    
public:
    TicTacServer( const std::string& addr, const std::string& port ) : TcpServer( addr, port ) {}
    
    virtual std::shared_ptr<TcpClientSession> createSession( boost::asio::ip::tcp::socket&& socket ) override
    {
        auto ptr = std::make_shared<TicTacClientSession>( *this, std::move(socket) );
        return ptr->shared_from_this();
    }
    
    virtual bool addClient( ClientName& clientName, const std::weak_ptr<TicTacClientSession>& session, std::string& errorText ) override
    {
        LOG( "TicTacServer::addClient: " << clientName );
        if ( auto it = m_clientMap.find( clientName ); it != m_clientMap.end() )
        {
            errorText = "client with same name already exists";
            return false;
        }
        
        m_clientMap[clientName] = session;
        return true;
    }
    
    virtual void sendPlayerListToAll() override
    {
        auto playerList = getPlayerListResponse();
        
        for( auto [clientName,sessionPtr] : m_clientMap )
        {
            if ( auto session = sessionPtr.lock(); session )
            {
                session->write( playerList );
            }
        }
    }
    
    virtual std::string getPlayerListResponse() override
    {
        std::string response = "PlayerList";
        
        for( const auto& [key,session] : m_clientMap )
        {
            response += "," + key;
        }
        response += ";";
        return response;
    }
    
    virtual bool sendInvitaion( std::string senderPlayerName, std::string playerName, std::string& outErrorText ) override
    {
        auto it = m_clientMap.find(playerName);
        if ( it != m_clientMap.end() )
        {
            outErrorText = "no player with name: " + playerName;
            return false;
        }
        
        if ( auto session = it->second.lock(); session )
        {
            std:: string message = SMT_INVITITAION + "," + senderPlayerName + ";";
            session->write( message );
            return true;
        }
        
        outErrorText = "player is off line: " + playerName;
        return false;
    }
    
    virtual bool sendInvitaionAccepted( bool isAccepted, std::string senderPlayerName, std::string playerName, std::string& outErrorText ) override
    {
        auto it = m_clientMap.find(playerName);
        if ( it != m_clientMap.end() )
        {
            outErrorText = "no player with name: " + playerName;
            return false;
        }
        
        if ( auto session = it->second.lock(); session )
        {
            std::string message;
            if ( isAccepted )
            {
                message = (SMT_INVITITAION_ACCEPTED) + "," + senderPlayerName + ";";
            }
            else
            {
                message = (SMT_INVITITAION_REJECTED) + "," + senderPlayerName + ";";
            }
            session->write( message );
            return true;
        }
        
        outErrorText = "player is off line: " + playerName;
        return false;
    }

    virtual bool sendStep( std::string playerName, std::string x_0, std::string x, std::string y ) override
    {
        auto it = m_clientMap.find(playerName);
        if ( it != m_clientMap.end() )
        {
            return false;
        }
        
        if ( auto session = it->second.lock(); session )
        {
            std::string message;
            message = (SMT_ON_STEP) + "," + playerName + "," + x_0 + "," + x + "," + y + ";";
            session->write( message );
            return true;
        }
        
        return false;
    }
    
    virtual bool sendCloseGame( std::string playerName, std::string otherPlayerName )
    {
        auto it = m_clientMap.find(playerName);
        if ( it != m_clientMap.end() )
        {
            return false;
        }
        
        if ( auto session = it->second.lock(); session )
        {
            std::string message;
            message = (SMT_GAME_CLOSED) + "," + otherPlayerName + ";";
            session->write( message );
            return true;
        }
        
        return false;
    }
};

} // namespace tic_tac {
