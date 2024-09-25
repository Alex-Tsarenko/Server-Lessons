#include "TicTacTcpServer.h"
#include "DbgTicTacClient.h"

// lvalue = rvalue (movable)
// rvalue = std::move(lvalue)

int main()
{
#ifndef STANDALONE_TEST
    tic_tac::TicTacServer server( "0.0.0.0", "15001" );
    server.run();
#else
    tic_tac::TicTacServer server( "127.0.0.1", "15001" );
    std::thread( [&server] { server.run(); }).detach();
    
    usleep(1000);

    std::thread( []
    {
        tic_tac::DbgTicTacClient client( "Player0" );
        client.run( "127.0.0.1", "15001" );
    }).detach();
    
    //usleep(1000);

    tic_tac::DbgTicTacClient client( "Player1" );
    client.run( "127.0.0.1", "15001" );
    
    sleep(1000);
    
    return 10;
#endif
}
