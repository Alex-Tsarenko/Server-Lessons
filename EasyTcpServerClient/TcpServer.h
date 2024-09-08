#include <boost/asio.hpp>
#include <iostream>
#include <ostream>
#include <strstream>
#include <optional>

#include <boost/algorithm/string.hpp>

#include "Logs.h"

class TcpClientSession: public std::enable_shared_from_this<TcpClientSession>
{
protected:
    boost::asio::ip::tcp::socket m_socket;
    std::string                  m_request;
    
public:
    TcpClientSession( boost::asio::ip::tcp::socket&& socket ) : m_socket( std::move(socket) )
    {
    }
    
    virtual void onMessage( const std::string& message ) // = 0; for debugging
    {
        LOG( "TcpClientSession::onMessage: " << message );
    }
    
    void start()
    {
        write( "Hi;" );
        read();
    }
    
    void read()
    {
        m_request.clear();
        
        boost::asio::async_read_until( m_socket, boost::asio::dynamic_buffer(m_request), ';',
            [self=shared_from_this()] ( auto error, size_t dataSize )
        {
            if ( error )
            {
                LOG_ERR( "TcpClientSession read error: " << error.message() );
                return;
            }
            
            LOG( "TcpClientSession read request: " << self->m_request );
            
            self->m_request.pop_back();
            self->onMessage( self->m_request );
        });
    }

    void write( const std::string& response )
    {
        m_socket.async_send( boost::asio::mutable_buffer( (void*)response.c_str(), response.size() ),
            [self=shared_from_this()] ( auto error, auto sentSize )
        {
            if (error)
            {
                LOG_ERR( "TcpClientSession async_send error: " << error.message() );
            }
        });
    }
};

class TcpServer
{
    boost::asio::io_context                         m_context;
    boost::asio::ip::tcp::endpoint                  m_endpoint;
    std::optional<boost::asio::ip::tcp::acceptor>   m_acceptor;

    boost::asio::ip::tcp::socket     m_socket;

public:
    TcpServer( const std::string& addr, const std::string& port )
      :
        m_context(),
        m_socket(m_context)
    {
        try
        {
            boost::asio::ip::tcp::resolver resolver(m_context);
            m_endpoint = *resolver.resolve( addr, port ).begin();

            m_acceptor = boost::asio::ip::tcp::acceptor( m_context, m_endpoint );
        }
        catch( std::runtime_error& e ) {
            LOG("TcpServer exception: " << e.what() )
            LOG("??? Port already in use ???" )
            exit(0);
        }
    }
    
    void run()
    {
        asyncAccept();
        m_context.run();
    }

    void shutdown()
    {
        m_context.stop();
    }
    
    void asyncAccept()
    {
        m_acceptor->async_accept( m_socket, m_endpoint, [this] (auto errorCode)
        {
            if (errorCode)
            {
                LOG_ERR( "async_accept error: " << errorCode.message() );
            }
            else
            {
                auto session = createSession( std::move(m_socket) );
                session->start();
                asyncAccept();
            }
        });
    }
    
    virtual std::shared_ptr<TcpClientSession> createSession( boost::asio::ip::tcp::socket&& )
    {
        return std::make_shared<TcpClientSession>( std::move(m_socket) );
    }
};

