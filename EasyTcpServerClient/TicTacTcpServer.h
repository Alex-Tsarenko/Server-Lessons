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
    virtual std::string getPlayerListResponse( const std::string requesterName ) = 0;

};

// Session derived from TCP session
// It is from applied/subject level of the program
// It related to subject logic
//
class TicTacClientSession: public TcpClientSession
{
    ITicTacServer& m_ticTacServer;
    std::string    m_playerName;

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
            read();
        }
        else if ( command == "Bye" )
        {
            //read();
        }
        else if ( command == "GetPlayers" )
        {
            std::string response = m_ticTacServer.getPlayerListResponse( m_playerName );

            write( response );
            read();
        }
        else if ( command == "Turn" )
        {
        }
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
    
    std::string getPlayerListResponse( const std::string requesterName ) override
    {
        std::string response = "PlayerList";
        
        for( const auto& [key,session] : m_clientMap )
        {
            if ( requesterName != key )
            {
                response += "," + key;
            }
        }
        return response;
    }
};

