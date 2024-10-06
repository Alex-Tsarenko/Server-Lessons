#include <boost/asio.hpp>
#include <iostream>
#include <ostream>
#include <strstream>
#include <optional>

#include <boost/algorithm/string.hpp>

#include "TicTacClientPackets.h"
#include "TicTacServerPackets.h"
#include "TicTacPacketUtils.h"
#include "Logs.h"

class TicTacSession/*: public std::enable_shared_from_this<TicTacSession>*/
{
public:
    TicTacSession()
    {
    }

    void onPacketReceived( const std::vector<uint8_t>& buffer )
    {
        tic_tac::ClientPacketType packetType = static_cast<tic_tac::ClientPacketType>( buffer[0] | (buffer[1] << 8) );
        
        const uint8_t* bufferPtr = buffer.data() + 2;
        size_t bufferSize        = buffer.size() - 2;
        switch( packetType )
        {
            case tic_tac::cpt_hi:
            {
                tic_tac::ClientPacketHi packet = readPacket<tic_tac::ClientPacketHi>( bufferPtr, bufferSize );
                LOG("");
                break;
            }
            default:
        }
    }
};

class TicTacServer
{
public:
    TicTacServer() {}
};

