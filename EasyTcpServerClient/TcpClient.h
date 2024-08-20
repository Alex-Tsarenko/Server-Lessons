#pragma once

class IClient
{
protected:
    virtual void onMessageReceived( const std::string& message ) = 0;
};

class TcpClient: protected IClient
{
    boost::asio::io_context         m_context;
    boost::asio::ip::tcp::socket    m_socket;

    std::vector<char>               m_sendMessage;
    
public:
    TcpClient() : m_context(), m_socket(m_context) {}
    
    void write( const std::string& message )
    {
        m_sendMessage.resize( message.size()+1 );
        std::memcpy( &m_sendMessage[0], message.c_str(), message.size()+1 );
                    
        boost::system::error_code ec;
        LOG( "message: " << message );
        boost::asio::write( m_socket, boost::asio::buffer(m_sendMessage), ec);
        //size_t sendLen = m_socket.send( boost::asio::buffer(m_sendMessage), 0, ec );
        if (ec)
        {
            LOG_ERR( "write error: " << ec.message() );
        }
    }
    
    void run( std::string address, std::string port )
    {
        try
        {
            boost::asio::ip::tcp::resolver  resolver(m_context);
            auto endpoints = resolver.resolve( address, port );
            
            boost::asio::connect( m_socket, endpoints );

            for(;;)
            {
                std::string response;
                boost::system::error_code ec;
                boost::asio::read_until( m_socket, boost::asio::dynamic_buffer(response), "\0", ec );
                if ( ec )
                {
                    LOG_ERR( "Client error: read_until error: " << this << " " << ec.message() );
                    if ( ec == boost::asio::error::eof )
                    {
                        return;
                    }
                }
                LOG( "Client received response: " << response );
                onMessageReceived( response );
            }
            
        }
        catch( std::runtime_error& exception )
        {
            LOG_ERR( "Client exception: " << exception.what() );
        }
        catch( ... )
        {
            LOG_ERR( "Client exception: ..." );
        }
    }
    
};
