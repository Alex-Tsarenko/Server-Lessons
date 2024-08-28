#pragma once

class IClient
{
protected:
    virtual void onMessageReceived( std::string& message ) = 0;
};

class TcpClient: protected IClient
{
    boost::asio::io_context         m_context;
    boost::asio::ip::tcp::socket    m_socket;

    std::vector<char>               m_sendMessage;
    
public:
    TcpClient() : m_context(), m_socket(m_context) {}
    virtual ~ TcpClient() = default;
    
    void write( const std::string& message )
    {
        m_sendMessage.resize( message.size()+1 );
        std::memcpy( &m_sendMessage[0], message.c_str(), message.size() );
        m_sendMessage.back() = ';';

        boost::system::error_code ec;
        LOG( "message: " << message );
        boost::asio::write( m_socket, boost::asio::buffer(m_sendMessage), ec);
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
                boost::asio::read_until( m_socket, boost::asio::dynamic_buffer(response), ";", ec );
                if (response.back() == '\0')
                {
                    response.pop_back();
                }
                if (response.back() == ';')
                {
                    response.pop_back();
                }

                std::vector<std::string> messages;
                boost::split( messages, response, boost::is_any_of(",;") );
                
                LOG_ERR( "response: " << response );
                LOG_ERR( "messages.size(): " << messages.size() );

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
