#include "SteamServerWrapper.h"

USteamServerWrapper::USteamServerWrapper()
	: ConnectionPort(0)
	, Ping(0)
	, Players(0)
	, MaxPlayers(0)
	, BotPlayers(0)
	, bPassword(false)
	, bSecure(false)
	, ServerVersion(0)
{
}

void USteamServerWrapper::Initialize(gameserveritem_t* pGameServerItem)
{
	IPAddress = ConvertIPToString(pGameServerItem->m_NetAdr.GetIP());
	ConnectionPort = pGameServerItem->m_NetAdr.GetConnectionPort();
	Ping = pGameServerItem->m_nPing;
	Map = FString(pGameServerItem->m_szMap);
	GameDescription = FString(pGameServerItem->m_szGameDescription);
	Players = pGameServerItem->m_nPlayers;
	MaxPlayers = pGameServerItem->m_nMaxPlayers;
	BotPlayers = pGameServerItem->m_nBotPlayers;
	bPassword = pGameServerItem->m_bPassword;
	bSecure = pGameServerItem->m_bSecure;
	ServerVersion = pGameServerItem->m_nServerVersion;
	ServerName = FString(pGameServerItem->GetName());
	ServerString = FString(pGameServerItem->m_szGameTags);
	SteamID = pGameServerItem->m_steamID.ConvertToUint64();
}

FString USteamServerWrapper::GetName() const
{
	return ServerName;
}

FString USteamServerWrapper::GetMap() const
{
	return Map;
}

FString USteamServerWrapper::GetPlayerCount() const
{
	return FString::Printf(TEXT("%d/%d"), Players, MaxPlayers);
}

FString USteamServerWrapper::GetDisplayString() const
{
	return ServerString;
}

FString USteamServerWrapper::GetIP() const
{
	return IPAddress;
}

int32 USteamServerWrapper::GetPort() const
{
	return ConnectionPort;
}

int32 USteamServerWrapper::GetPing() const
{
	return Ping;
}

int64 USteamServerWrapper::GetSteamID() const
{
	return SteamID;
}

FString USteamServerWrapper::ConvertIPToString(uint32 IP) const
{
	uint8 A = (IP & 0xFF000000) >> 24;
	uint8 B = (IP & 0x00FF0000) >> 16;
	uint8 C = (IP & 0x0000FF00) >> 8;
	uint8 D = (IP & 0x000000FF);

	return FString::Printf(TEXT("%d.%d.%d.%d"), A, B, C, D);
}

FString USteamServerWrapper::ConnectionIP() const
{
	return FString::Printf(TEXT("%s:%d"), *IPAddress, ConnectionPort);
}