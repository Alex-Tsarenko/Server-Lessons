#pragma once

#include <iostream>
#include <vector>

namespace tic_tac
{

enum PacketType : uint16_t 
{
    // from client to client
    cpt_undefined = 0,
    cpt_hi,
    cpt_invite,
    cpt_invitation_responce,
    cpt_step,
    cpt_status,
    
    // from server to server
    spt_already_exists = 100,
    spt_player_list,

};

enum ClientStatus : uint16_t
{
    cst_not_accesible,
    cst_accesible,
    cst_gaming,
    cst_offline,
};

struct PacketHi
{
    std::string m_playerName;

    PacketHi( std::string playerName ) : m_playerName(playerName) {}
    
    constexpr static PacketType packetType() { return cpt_hi; }
    
    template<class ExecutorT>
    void fields( ExecutorT& executor )
    {
        executor(m_playerName);
    }
};

struct PacketStep
{
    bool        m_isX;
    uint16_t    m_x;
    uint16_t    m_y;
    
    PacketStep( bool isX, uint16_t x, uint16_t y ) : m_isX(isX), m_x(x), m_y(y) {}
    
    constexpr static PacketType packetType() { return cpt_step; }
    
    template<class ExecutorT>
    void fields( const ExecutorT& executor )
    {
        executor( m_isX, m_x, m_y );
    }
};

struct PacketInvite
{
    PacketInvite() {}
    
    constexpr static PacketType packetType() { return cpt_invite; }
    
    template<class ExecutorT>
    void fields( const ExecutorT& executor )
    {
    }
};

struct PacketInvitationResponce
{
    bool        m_accepted;
    
    PacketInvitationResponce( bool accepted ) : m_accepted(accepted) {}
    
    constexpr static PacketType packetType() { return cpt_invitation_responce; }
    
    template<class ExecutorT>
    void fields( const ExecutorT& executor )
    {
        executor( m_accepted );
    }
};

struct PacketClientStatus
{
    std::string     m_myName;
    ClientStatus    m_status;
    
    PacketClientStatus( std::string myName, ClientStatus status ) : m_myName(myName), m_status(status) {}
    
    constexpr static PacketType packetType() { return cpt_status; }
    
    template<class ExecutorT>
    void fields( ExecutorT& executor )
    {
        executor( m_myName, (uint16_t&)m_status );
    }
};

struct ServerPacketPlayerAlreadyExists
{
    constexpr static PacketType packetType() { return spt_already_exists; }
    
    template<class ExecutorT>
    void fields( const ExecutorT& executor )
    {
    }
};

struct PlayerStatus
{
    std::string     m_playerName;
    ClientStatus    m_status = cst_not_accesible;
    
    template<class ExecutorT>
    void fields( ExecutorT& executor )
    {
        executor( m_playerName, reinterpret_cast<uint16_t&>( m_status ) );
    }
    
    template<class ExecutorT>
    void fields( ExecutorT& executor ) const
    {
        executor( m_playerName, reinterpret_cast<const uint16_t&>( m_status ) );
    }
};

struct ServerPacketPlayerList
{
    std::vector<PlayerStatus> m_playerList;
    
    ServerPacketPlayerList() {}
    ServerPacketPlayerList( const std::vector<PlayerStatus>& playerList ) : m_playerList(playerList) {}

    constexpr static PacketType packetType()  { return spt_player_list; }
    
    template<class ExecutorT>
    void fields( ExecutorT& executor )
    {
        executor( m_playerList );
    }
};




}
