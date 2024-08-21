#include "TicTacTcpServer.h"
#include "DbgTicTacClient.h"

// lvalue = rvalue (movable)
// rvalue = std::move(lvalue)

int main()
{
    TicTacServer server( "127.0.0.1", "15001" );
    std::thread( [&server] { server.run(); }).detach();
    
    usleep(1000);

    std::thread( []
    {
        DbgTicTacClient client( "Player0" );
        client.run( "127.0.0.1", "15001" );
    }).detach();
    
    //usleep(1000);

    DbgTicTacClient client( "Player1" );
    client.run( "127.0.0.1", "15001" );
    
    sleep(1000);
    
    return 10;
}
