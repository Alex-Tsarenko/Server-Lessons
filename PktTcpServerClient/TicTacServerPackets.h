#pragma once
//
//#include <iostream>
//#include <vector>
#include "TicTacClientPackets.h"
//
//namespace tic_tac
//{
//
//enum ServerPacketType : uint16_t {
//    spt_undefined = 200,
//    spt_already_exists,
//    spt_player_list,
//    spt_invitation_from,
//    spt_invitation_responce,
//    spt_step,
//    spt_player_status,
//};
//
//struct PlayerStatus
//{
//    std::string     m_playerName;
//    ClientStatus    m_status = cst_not_accesible;
//};
//
//struct ServerPacketBase
//{
//    constexpr ServerPacketType packetType() const { return spt_undefined; }
//};
//
//struct PlayerAlreadyExists
//{
//    std::string     m_playerName;
//    
//    constexpr ServerPacketType packetType() const { return spt_already_exists; }
//    
//    template<class ExecutorT>
//    void fields( const ExecutorT& executor )
//    {
//        executor( m_playerName );
//    }
//};
//
//struct ServerPacketPlayerList: public ServerPacketBase
//{
//    std::vector<PlayerStatus> m_playerList;
//    
//    ServerPacketPlayerList() {}
//    
//    constexpr ServerPacketType packetType() const { return spt_player_list; }
//    
//    template<class ExecutorT>
//    void fields( const ExecutorT& executor )
//    {
//        executor( m_playerList );
//    }
//    
//};
//
//
//struct ServerPktInvitationFrom: public ServerPacketBase
//{
//    std::string m_otherPlayerName;
//    
//    ServerPktInvitationFrom( std::string otherPlayerName ) : m_otherPlayerName(otherPlayerName) {}
//    
//    constexpr ServerPacketType packetType() const { return spt_invitation_from; }
//    
//    template<class ExecutorT>
//    void fields( const ExecutorT& executor )
//    {
//        executor( m_otherPlayerName );
//    }
//    
//};
//
//struct ServerPktInvitationResponce: public ServerPacketBase
//{
//    std::string m_otherPlayerName;
//    bool        m_isAccepted;
//    
//    ServerPktInvitationResponce( std::string otherPlayerName, bool isAccepted )
//      :
//        m_otherPlayerName(otherPlayerName),
//        m_isAccepted(isAccepted)
//      {}
//    
//    constexpr ServerPacketType packetType() const { return spt_invitation_responce; }
//    
//    template<class ExecutorT>
//    void fields( const ExecutorT& executor )
//    {
//        executor( m_otherPlayerName, m_isAccepted );
//    }
//};
//
//struct ServerPktStep: public ServerPacketBase
//{
//    std::string m_otherPlayerName;
//    bool        m_isX;
//    uint16_t    m_x;
//    uint16_t    m_y;
//
//    ServerPktStep( std::string otherPlayerName, bool isX, uint16_t x, uint16_t y )
//      :
//        m_otherPlayerName(otherPlayerName),
//        m_isX(isX),
//        m_x(y),
//        m_y(y)
//      {}
//    
//    constexpr ServerPacketType packetType() const { return spt_step; }
//    
//    template<class ExecutorT>
//    void fields( const ExecutorT& executor )
//    {
//        executor( m_otherPlayerName, m_isX, m_x, m_y );
//    }
//};
//
//struct ServerPktPlayerStatus: public ServerPacketBase
//{
//    std::string     m_playerName;
//    ClientStatus    m_status;
//
//    ServerPktPlayerStatus( std::string playerName, ClientStatus status )
//      :
//        m_playerName(playerName),
//        m_status(status)
//      {}
//    
//    constexpr ServerPacketType packetType() const { return spt_player_status; }
//    
//    template<class ExecutorT>
//    void fields( const ExecutorT& executor )
//    {
//        executor( m_playerName, m_status );
//    }
//};
//
//struct Envelope
//{
//    std::string m_playerName;
//    std::string m_packet;
//    
//    Envelope( uint16_t envelopeType, std::string playerName = {}, std::string&& packet )
//      :
//        m_envelopeType( envelopeType ),
//        m_playerName( playerName ),
//        m_packet( std::move(packet) )
//    {}
//    
//    template<class ExecutorT>
//    void fields( const ExecutorT& executor )
//    {
//        executor( m_envelopeType, m_playerName, m_packet );
//    }
//}
