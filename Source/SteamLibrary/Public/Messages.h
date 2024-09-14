#include "ER.h"

#ifndef MESSAGES_H
#define MESSAGES_H

#pragma pack( push, 1 )

// Network message types
enum EMessage
{
	// Server messages
	k_EMsgServerBegin = 0,
	k_EMsgServerSendInfo = k_EMsgServerBegin+1,
	k_EMsgServerFailAuthentication = k_EMsgServerBegin+2,
	k_EMsgServerPassAuthentication = k_EMsgServerBegin+3,
	k_EMsgServerUpdateWorld = k_EMsgServerBegin+4,
	k_EMsgServerExiting = k_EMsgServerBegin+5,
	k_EMsgServerPingResponse = k_EMsgServerBegin+6,

	// Client messages
	k_EMsgClientBegin = 500,
	k_EMsgClientBeginAuthentication = k_EMsgClientBegin+2,
	k_EMsgClientSendLocalUpdate = k_EMsgClientBegin+3,

	// P2P authentication messages
	k_EMsgP2PBegin = 600, 
	k_EMsgP2PSendingTicket = k_EMsgP2PBegin+1,

	// voice chat messages
	k_EMsgVoiceChatBegin = 700, 
	//k_EMsgVoiceChatPing = k_EMsgVoiceChatBegin+1,	// deprecated keep alive message
	k_EMsgVoiceChatData = k_EMsgVoiceChatBegin+2,	// voice data from another player



	// force 32-bit size enum so the wire protocol doesn't get outgrown later
	k_EForceDWORD  = 0x7fffffff, 
};

enum EDisconnectReason
{
	k_EDRClientDisconnect = k_ESteamNetConnectionEnd_App_Min + 1,
	k_EDRServerClosed = k_ESteamNetConnectionEnd_App_Min + 2,
	k_EDRServerReject = k_ESteamNetConnectionEnd_App_Min + 3,
	k_EDRServerFull = k_ESteamNetConnectionEnd_App_Min + 4,
	k_EDRClientKicked = k_ESteamNetConnectionEnd_App_Min + 5
};

// Msg from the server to the client when refusing a connection
struct MsgServerFailAuthentication_t
{
	MsgServerFailAuthentication_t() : m_dwMessageType(  k_EMsgServerFailAuthentication ) {}
	DWORD GetMessageType() { return m_dwMessageType; }
private:
	const DWORD m_dwMessageType;
};

struct MsgClientSendLocalUpdate_t
{
	MsgClientSendLocalUpdate_t() : m_dwMessageType( LittleDWord( k_EMsgClientSendLocalUpdate ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }
	
	//void SetShipPosition( uint32 uPos ) { m_uShipPosition = LittleDWord( uPos ); }
	ClientERUpdateData_t *AccessUpdateData() { return &m_ClientUpdateData; }

private:
	const DWORD m_dwMessageType;

	//uint32 m_uShipPosition;
	ClientERUpdateData_t m_ClientUpdateData;
};

// Msg from client to server when initiating authentication
struct MsgClientBeginAuthentication_t
{
	MsgClientBeginAuthentication_t() : m_dwMessageType( LittleDWord( k_EMsgClientBeginAuthentication ) ) {}
	DWORD GetMessageType() { return LittleDWord( m_dwMessageType ); }

	void SetToken( const char *pchToken, uint32 unLen ) { m_uTokenLen = LittleDWord( unLen );  memcpy(m_rgchToken, pchToken, FMath::Min(unLen, static_cast<uint32>(sizeof(m_rgchToken)))); }
	uint32 GetTokenLen() { return LittleDWord( m_uTokenLen ); }
	const char *GetTokenPtr() { return m_rgchToken; }

	void SetSteamID( uint64 ulSteamID ) { m_ulSteamID = LittleQWord( ulSteamID ); }
	uint64 GetSteamID() { return LittleQWord( m_ulSteamID ); }

private:
	const DWORD m_dwMessageType;
	
	uint32 m_uTokenLen;
#ifdef USE_GS_AUTH_API
	char m_rgchToken[1024];
#endif
	uint64 m_ulSteamID;
};

#pragma pack( pop )

#endif MESSAGES_H