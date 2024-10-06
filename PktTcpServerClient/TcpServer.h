#include <boost/asio.hpp>
#include <iostream>
#include <ostream>
#include <strstream>
#include <optional>

#include <boost/algorithm/string.hpp>

#include "Logs.h"

template<class AppliedSessionT>
class TcpClientSession: public std::enable_shared_from_this<TcpClientSession<AppliedSessionT>>, public AppliedSessionT
{
protected:
    boost::asio::ip::tcp::socket m_socket;

    uint16_t                     m_dataLength;
    std::vector<uint8_t>         m_packetData;
    
public:
    TcpClientSession( boost::asio::ip::tcp::socket&& socket )
        : m_socket( std::move(socket) )
    {
    }

    void write( const uint8_t* response, size_t dataSize )
    {
        m_socket.async_send( boost::asio::buffer( response, dataSize ),
            [self=this->shared_from_this(),response] ( auto error, auto sentSize )
        {
            LOG_ERR( "TcpClientSession sentSize: " << sentSize );
            delete response;
            if (error)
            {
                LOG_ERR( "TcpClientSession async_send error: " << error.message() );
            }
        });
    }

    void readPacketHeader()
    {
        //m_packetData.clear();
        
        boost::asio::async_read( m_socket, boost::asio::buffer(&m_dataLength, sizeof(m_dataLength)), [self=this->shared_from_this()] ( auto error, auto bytes_transferred )
        {
            self->doReadPacketHeader( error, bytes_transferred );
        });
    }
    
    void doReadPacketHeader( boost::system::error_code error, size_t bytes_transferred )
    {
        if ( error )
        {
            LOG_ERR( "TcpClientSession read error: " << error.message() );
            //connectionLost( error );
            return;
        }
        if ( bytes_transferred != sizeof(m_dataLength) )
        {
            LOG_ERR( "TcpClientSession read error (m_dataLength): " << bytes_transferred << " vs " << sizeof(m_dataLength) );
            //connectionLost( error );
            return;
        }
        
        LOG( "received: " << m_dataLength );

        m_packetData.resize( m_dataLength );
        boost::asio::async_read( m_socket, boost::asio::buffer(m_packetData.data(), m_dataLength-2 ),
                                [self=this->shared_from_this()] ( auto error, auto bytes_transferred )
        {
            self->readPacketData( error, bytes_transferred );
        });
    }
    
    void readPacketData( boost::system::error_code error, size_t bytes_transferred )
    {
        if ( error )
        {
            LOG_ERR( "TcpClientSession read error: " << error.message() );
            //connectionLost( error );
            return;
        }
        if ( bytes_transferred != m_dataLength-2 )
        {
            LOG_ERR( "TcpClientSession read error (bytes_transferred): " << bytes_transferred << " vs "  << m_dataLength );
            //connectionLost( error );
            return;
        }
        
        //LOG( "received: " << std::string( m_packetData.data(), m_packetData.size() ) );
        
        AppliedSessionT::onPacketReceived( m_packetData );

        // Read next packet
        readPacketHeader();
    }
};

template<class AppliedServerT,class AppliedSessionT>
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
                boost::asio::socket_base::keep_alive option(true);
                m_socket.set_option(option);
                
                auto session = createSession( std::move(m_socket) );
                session->readPacketHeader();
                asyncAccept();
            }
        });
    }
    
    std::shared_ptr<TcpClientSession<AppliedSessionT>> createSession( boost::asio::ip::tcp::socket&& socket )
    {
        return std::make_shared<TcpClientSession<AppliedSessionT>>( std::move(socket) );
    }
};

