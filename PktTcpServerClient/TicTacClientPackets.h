#pragma once

#include <iostream>
#include <vector>

namespace tic_tac
{

enum ClientPacketType : uint16_t {
    cpt_undefined = 100,
    cpt_hi,
    cpt_invite,
    cpt_invitation_responce,
    cpt_step,
    cpt_status,
};

enum ClientStatus : uint16_t {
    cst_not_accesible,
    cst_accesible,
    cst_gaming,
    cst_offline,
};

struct ClientPacketBase
{
    constexpr static ClientPacketType packetType() { return cpt_undefined; }
};

struct ClientPacketHi: public ClientPacketBase
{
    std::string m_playerName;
    
    ClientPacketHi() {}
    ClientPacketHi( std::string playerName ) : m_playerName(playerName) {}
    
    constexpr static ClientPacketType packetType() { return cpt_hi; }
    
    template<class ExecuterT>
    void fields( ExecuterT& executer )
    {
        executer( m_playerName );
    }
};

struct ClientPacketStep: public ClientPacketBase
{
    std::string m_partnerName;
    bool        m_isX;
    uint16_t    m_x;
    uint16_t    m_y;
    
//    ClientPacketStep() {}
    ClientPacketStep( std::string partnerName, bool isX, uint16_t x, uint16_t y ) : m_partnerName(partnerName), m_isX(isX), m_x(x), m_y(y) {}
    
    constexpr static ClientPacketType packetType() { return cpt_step; }
    
    template<class ExecuterT>
    void fields( const ExecuterT& executer )
    {
        executer( m_partnerName, m_isX, m_x, m_y );
    }
};

struct ClientPacketInvite: public ClientPacketBase
{
    std::string m_partnerName;
    
    ClientPacketInvite( std::string partnerName ) : m_partnerName(partnerName) {}
    
    constexpr static ClientPacketType packetType() { return cpt_invite; }
    
    template<class ExecuterT>
    void fields( const ExecuterT& executer )
    {
        executer( m_partnerName );
    }
};

struct ClientPacketInvitationResponce: public ClientPacketBase
{
    std::string m_partnerName;
    bool        m_accepted;
    
    ClientPacketInvitationResponce( std::string partnerName, bool accepted ) : m_partnerName(partnerName), m_accepted(accepted) {}
    
    constexpr static ClientPacketType packetType() { return cpt_invitation_responce; }
    
    template<class ExecuterT>
    void fields( const ExecuterT& executer )
    {
        executer( m_partnerName, m_accepted );
    }
};

struct ClientPacketStatus: public ClientPacketBase
{
    std::string     m_myName;
    ClientStatus    m_status;
    
    ClientPacketStatus( std::string myName, ClientStatus status ) : m_myName(myName), m_status(status) {}
    
    constexpr static ClientPacketType packetType() { return cpt_status; }
    
    template<class ExecuterT>
    void fields( ExecuterT& executer )
    {
        executer( m_myName, m_status );
    }
};

}
