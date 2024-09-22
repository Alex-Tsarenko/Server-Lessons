#pragma once

class IClient
{
protected:
    virtual void onMessageReceived( const std::string& message ) = 0;
    virtual std::string clientName() const = 0;
};

class TcpClient: protected IClient
{
    std::vector<char>               m_sendMessage;

    boost::asio::io_context         m_context;
    boost::asio::ip::tcp::socket    m_socket;

protected:
    std::mutex                      m_mutex;
    
public:
    TcpClient() : m_context(), m_socket(m_context) {}
    virtual ~ TcpClient() = default;
    
    void write( const std::string& message )
    {
        LOG( ">>> sendMessage: (" << clientName().c_str() << "): " << message.c_str() );
        m_sendMessage.resize( message.size()+1 );
        std::memcpy( &m_sendMessage[0], message.c_str(), message.size() );
        m_sendMessage.back() = ';';

        boost::system::error_code ec;
        boost::asio::write( m_socket, boost::asio::buffer(m_sendMessage), ec);
        if (ec)
        {
            LOG_ERR( "write error: " << ec.message().c_str() );
        }
    }

    void closeSocket() { m_socket.close(); }
    
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
                boost::asio::read_until( m_socket, boost::asio::dynamic_buffer(response), ";", ec );

                if ( ec )
                {
                    LOG_ERR( "Client error: read_until error: " << this << " " << ec.message().c_str() );
                    if ( ec == boost::asio::error::eof || ec == boost::asio::error::bad_descriptor )
                    {
                        return;
                    }
                }

                LOG( "response: (" << clientName().c_str() << ") \'" << response.c_str() << '\'');

                char* ptr = const_cast<char*>( response.c_str() );
                auto* responseEnd  = response.c_str()+response.size();
                for( char* end = ptr ; end < responseEnd; end++ )
                {
                    if ( *end == ';' )
                    {
                        *end = 0;
                        LOG( "message: (" << clientName().c_str() << ")" << ptr );
                        {
                            onMessageReceived( std::string(ptr) );
                        }
                        ptr = end+1;
                    }
                }
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
