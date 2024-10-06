#include "TcpServer.h"
#include "TicTacServer.h"
#include "TicTacClient.h"
#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TicTacPacketUtils.h"

// lvalue = rvalue (movable)
// rvalue = std::move(lvalue)

int main()
{
//    tic_tac::ClientPacketStep packet0("Name",false, 1,2);
//    std::array<uint8_t,200> buffer;
//    writePacket( packet0, buffer.data(), buffer.size() );
//    
//    tic_tac::ClientPacketStep packet2 = readPacket<tic_tac::ClientPacketStep>( buffer.data(), buffer.size() );
//    
//    sleep(0);

    std::thread( []
    {
        TcpServer<TicTacServer,TicTacSession> server("0.0.0.0", "15001" );
        server.run();
    }).detach();
    
    
    auto client = std::make_shared<TcpClient<TicTacClientBase>>();
    
    std::thread clientThread( [&client]
    {
        client->run( "localhost", "15001" );
    });
    
    sleep(1);
    
    tic_tac::ClientPacketHi packet("Name");
    size_t tcpPacketSize;
    uint8_t* message = createOutgoingMessage( packet, tcpPacketSize );
    client->write( message, tcpPacketSize );

    sleep(100);

    return 0;
}
