// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define USE_GS_AUTH_API

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemSteam.h"
#include "OnlineSessionSettings.h"
THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
#include "Steam/steam_gameserver.h"
#include "Steam/isteamnetworkingsockets.h" 
#include "Steam/steamclientpublic.h"
#include "Messages.h"
THIRD_PARTY_INCLUDES_END
#include "ServerGameMode.generated.h"

UCLASS()
class ER_API AServerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AServerGameMode();

protected:
	virtual void StartPlay() override;
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaSeconds) override;
	void ReceiveNetworkData();
	
private:
	void InitSteamServer();
	void ShutdownSteamServer();

	/* Delegate called when session created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/* Delegate called when session started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	/** Handles to registered delegates for creating/starting a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);
	FString GetMapName() const;

	// void SendUpdatedServerDetailsToSteam();
	// void RemovePlayerFromServer(uint32 PlayerIndex, EDisconnectReason Reason);
	// bool SendDataToClient(uint32 PlayerIndex, char *Data, uint32 SizeOfData);
	// bool SendDataToPendingClient( uint32 uShipIndex, char *pData, uint32 nSizeOfData );
	// void OnClientBeginAuthentication(CSteamID steamIDClient, HSteamNetConnection connectionID, void* pToken, uint32 uTokenLen);
	// void OnAuthCompleted( bool bAuthSuccess, uint32 iPendingAuthIndex );
	
	
	// STEAM_GAMESERVER_CALLBACK( AServerGameMode, OnSteamServersConnected, SteamServersConnected_t );
	// STEAM_GAMESERVER_CALLBACK( AServerGameMode, OnSteamServersConnectFailure, SteamServerConnectFailure_t );
	// STEAM_GAMESERVER_CALLBACK( AServerGameMode, OnSteamServersDisconnected, SteamServersDisconnected_t );
	// STEAM_GAMESERVER_CALLBACK( AServerGameMode, OnPolicyResponse, GSPolicyResponse_t );
	// STEAM_GAMESERVER_CALLBACK( AServerGameMode, OnValidateAuthTicketResponse, ValidateAuthTicketResponse_t );
	// STEAM_GAMESERVER_CALLBACK( AServerGameMode, OnNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t );

	struct ClientConnectionData
	{
		bool m_bActive;
		CSteamID m_SteamIDUser;
		uint64 m_ulTickCountLastData;
		HSteamNetConnection m_hConn;

		ClientConnectionData() {
			m_bActive = false;
			m_ulTickCountLastData = 0;
			m_hConn = 0;
		}
	};

	bool bConnectedToSteam;
	uint32 PlayerCount;
	
	// Poll group and listen socket handles
	HSteamListenSocket ListenSocket;
	HSteamNetPollGroup PollGroup;

	// Vector to keep track of client connections
	ClientConnectionData ClientData[MAX_PLAYERS_PER_SERVER];
	
	// Vector to keep track of client connections which are pending auth
	ClientConnectionData PendingClientData[MAX_PLAYERS_PER_SERVER];
};
