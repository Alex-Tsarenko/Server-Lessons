#include "TcpServer.h"
#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TicTacPacketUtils.h"
#include "TicTacServer.h"
#include "TicTacClient.h"

#include "DbgUiClient.h"

// lvalue = rvalue (movable)
// rvalue = std::move(lvalue)

int main()
{
    std::thread( []
    {
        TcpServer< tic_tac::Server, tic_tac::Session > server("0.0.0.0", "15001" );
        server.run();
    }).detach();
    
    
    auto client = std::make_shared< TcpClient< tic_tac::Client<DbgUiClient> > >();
    
    std::thread clientThread( [&]
    {
        client->run( "localhost", "15001", "DbgClient1" );
    });
    
    usleep(100);

    auto client2 = std::make_shared< TcpClient< tic_tac::Client<DbgUiClient> > >();
    
    std::thread clientThread2( [&]
    {
        client2->run( "localhost", "15001", "DbgClient2" );
    });
    
    clientThread.join();

    return 0;
}
