#include "TcpServer.h"
#include "TicTacClient.h"
#include "TicTacPackets.h"

// lvalue = rvalue (movable)
// rvalue = std::move(lvalue)

int main()
{
    std::thread( []
    {
        TcpServer server("0.0.0.0", "15001" );
        server.run();
    }).detach();
    
    
    auto client = std::make_shared<TcpClient<TicTacClientBase>>();
    
    std::thread clientThread( [&client]
    {
        client->run( "localhost", "15001" );
    });
    
    sleep(1);
    
    char message[] = "001234567890";
    message[0] = 10;
    message[1] = 0;
    client->write( message, sizeof(message) );
    
    sleep(100);

    return 0;
}
