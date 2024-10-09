#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;

template<class T>
class TcpClient : public std::enable_shared_from_this<TcpClient<T>>, public T
{
    boost::asio::io_context m_context;
    
    tcp::resolver m_resolver;
    tcp::socket   m_socket;

    uint16_t               m_dataLength;
    std::vector<uint8_t>   m_packetData;

public:
    TcpClient() : T(),
        m_context(), m_resolver(m_context), m_socket(m_context)
    {
    }

    template<class ...Args>
    void run(  const std::string& host, const std::string& port, Args&... initParameters )
    {
        this->initClient( initParameters... );
        
        tcp::resolver::query query(host, port);
        m_resolver.async_resolve(query,
            [self = this->shared_from_this()](const boost::system::error_code& ec, tcp::resolver::iterator endpoint_iterator) {
                self->onResolve(ec, endpoint_iterator);
            });

        try
        {
            m_context.run();
        }
        catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
    }

    virtual void doWrite( const uint8_t* message, size_t size ) override
    {
        write( message, size );
    }
    
    void write( const uint8_t* message, size_t size ) {
        boost::asio::async_write(m_socket, boost::asio::buffer(message,size),
            [message,self = this->shared_from_this()](const boost::system::error_code& ec, std::size_t length)
        {
            delete [] message;
            if (!ec) {
                std::cout << "Sent message: " << length << " bytes\n";
            }
        });
    }

private:
    void onResolve(const boost::system::error_code& ec, tcp::resolver::iterator endpoint_iterator) {
        if (!ec) {
            // Try to connect to the resolved endpoint
            boost::asio::async_connect(m_socket, endpoint_iterator,
                [self = this->shared_from_this()](const boost::system::error_code& ec, tcp::resolver::iterator) {
                    self->onConnect(ec);
                });
        } else {
            std::cerr << "Error resolving: " << ec.message() << "\n";
        }
    }

    void onConnect(const boost::system::error_code& ec) {
        if (!ec) {
            std::cout << "Successfully connected to the server!\n";
            readPacketHeader();
        } else {
            std::cerr << "Error connecting: " << ec.message() << "\n";
        }
    }

    void readPacketHeader()
    {
        boost::asio::async_read( m_socket, boost::asio::buffer(&m_dataLength, sizeof(m_dataLength)), [self=this->shared_from_this()] ( auto error, auto bytes_transferred )
        {
            self->onReadPacketHeader( error, bytes_transferred );
        });
    }
    
    void onReadPacketHeader( boost::system::error_code error, size_t bytes_transferred )
    {
        //readPacketData( error, bytes_transferred );
        if ( error )
        {
            LOG_ERR( "TcpClient read error: " << error.message() );
            //connectionLost( error );
            return;
        }
        if ( bytes_transferred != sizeof(m_dataLength) )
        {
            LOG_ERR( "TcpClient read error (m_dataLength): " << bytes_transferred << " vs " << sizeof(m_dataLength) );
            //connectionLost( error );
            return;
        }
        
        LOG( "TcpClient received: " << m_dataLength );
        
        if ( m_dataLength == 0 || m_dataLength > 1024*1024 )
        {
            LOG_ERR( "TcpClient invalid dataLength: " << m_dataLength );
            //connectionLost( error );
            return;
        }

        
        m_packetData.resize( m_dataLength );
        boost::asio::async_read( m_socket, boost::asio::buffer(m_packetData.data(), m_dataLength),
                                [self=this->shared_from_this()] ( auto error, auto bytes_transferred )
        {
            self->readPacketData( error, bytes_transferred );
        });
    }
    
    void readPacketData( boost::system::error_code error, size_t bytes_transferred )
    {
        if ( error )
        {
            LOG_ERR( "TcpClient read error: " << error.message() );
            //connectionLost( error );
            return;
        }
        if ( bytes_transferred != m_dataLength )
        {
            LOG_ERR( "TcpClient read error (bytes_transferred): " << bytes_transferred << " vs "  << m_dataLength );
            //connectionLost( error );
            return;
        }
        
//        uint8_t* response = new uint8_t[14];
//        response[0] = 12;
//        response[0] = 0;
//        memcpy( response+2, "ba9876543210", 12 );
//        write( response, 14 );
        // onPacketReceived
        
        readPacketHeader();
    }

};

//int main() {
//    try {
//        boost::asio::io_service io_service;
//        auto client = std::make_shared<AsyncTCPClient>(io_service, "127.0.0.1", "8080");
//        io_service.run();
//    } catch (std::exception& e) {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }
//    return 0;
//}
