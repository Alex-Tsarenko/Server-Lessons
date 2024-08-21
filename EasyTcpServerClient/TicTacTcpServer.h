#include "TcpServer.h"
#include "Logs.h"
#include <map>

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

    virtual std::weak_ptr<TicTacClientSession> sendInvitaion( std::string senderPlayerName, std::string playerName, std::string& outErrorText ) = 0;

    //virtual bool onAcceptInvitaion( std::string senderPlayerName, std::string playerName, std::string& outErrorText ) = 0;
};


// Session derived from TCP session
// It is from applied/subject level of the program
// It related to subject logic
//
class TicTacClientSession: public TcpClientSession
{
    ITicTacServer& m_ticTacServer;
    std::string    m_playerName;

    // Game info
    bool  m_isX;
    
    bool                                m_waitingAcceptInitaion = false;
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

        if ( request.empty() || request.back() != '\0' )
        {
            LOG_ERR( "TcpClientSession bad request: " << request );
            return;
        }
        
        std::vector<std::string> tokens;
        boost::split( tokens, request, boost::is_any_of(",") );

        std::string command = tokens[0];
        
        if ( command == "Hello" )
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
                std::string response = "Failed," + errorText;
                write( response );
                return;
            }
            
            std::string response = "Ok";
            write( response );
            
            m_ticTacServer.sendPlayerListToAll();
        }
        else if ( command == "Invitation" )
        {
            if ( tokens.size() < 2 || m_waitingAcceptInitaion )
            {
                LOG_ERR( "TcpClientSession bad request (3): " << request );
                return;
            }
            
            auto& otherPlayerName = tokens[1];
            LOG( "otherPlayerName: " << otherPlayerName );

            std::string errorText;
            
            m_otherPlayer = m_ticTacServer.sendInvitaion( m_playerName, otherPlayerName, errorText );
            if ( m_otherPlayer.expired() )
            {
                std::string response = "Failed," + errorText;
                write( response );
                m_waitingAcceptInitaion = true;
            }
        }
        else if ( command == "AcceptInvitation" )
        {
            if ( auto otherPlayer = m_otherPlayer.lock(); otherPlayer )
            {
                otherPlayer->m_isX = false;
                m_isX = true;
                
                m_waitingAcceptInitaion = false;
                otherPlayer->m_waitingAcceptInitaion = false;

                std::string message = "AcceptedFrom," + m_playerName + ",1";
                otherPlayer->write( message );
            }
            else
            {
                m_waitingAcceptInitaion = false;
                std::string response = "PlayerOffLine";
                write( response );
            }
        }
        else if ( command == "Step" )
        {
            auto& isX = tokens[1];
            
            if ( (m_isX && isX != "X ") || (!m_isX && isX == "X ") )
            {
                std::string response = "Failed, invalid step (X or 0)";
                write( response );
                return;
            }
            
            if ( auto otherPlayer = m_otherPlayer.lock(); otherPlayer )
            {
                std::string message = "Step," + isX + "," + tokens[1] + "," + tokens[2];
                otherPlayer->write(message);
            }
        }
        else if ( command == "Bye" )
        {
            //read();
        }
        else if ( command == "PlayerStep" )
        {
        }
        else
        {
            LOG_ERR( "Unnkown message type: " << command );
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
            errorText == "client with same name already exists";
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
        return response;
    }
    
    virtual std::weak_ptr<TicTacClientSession> sendInvitaion( std::string senderPlayerName, std::string playerName, std::string& outErrorText ) override
    {
        auto it = m_clientMap.find(playerName);
        if ( it != m_clientMap.end() )
        {
            outErrorText = "no player with name: " + playerName;
            return {};
        }

        if ( auto session = it->second.lock(); session )
        {
            std:: string message = "InvitationFrom,"+senderPlayerName;
            session->write( message );
            return it->second;
        }

        outErrorText = "player is off line: " + playerName;
        return {};
    }
};

