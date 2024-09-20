// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerGameMode.h"

#include "Online/OnlineSessionNames.h"

AServerGameMode::AServerGameMode()
{
	// Set this game mode to be used in a dedicated server
	bUseSeamlessTravel = true;
	
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &AServerGameMode::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &AServerGameMode::OnStartOnlineGameComplete);
	
	// Enable tick
	PrimaryActorTick.bCanEverTick = true;
}

void AServerGameMode::StartPlay()
{
	Super::StartPlay();

	if (IsRunningDedicatedServer())
	{
		InitSteamServer();
	}
}

void AServerGameMode::BeginDestroy()
{
	if (IsRunningDedicatedServer())
	{
		ShutdownSteamServer();
	}
	
	Super::BeginDestroy();
}

void AServerGameMode::InitSteamServer()
{
	 uint32 ip = 0; // Use INADDR_ANY to bind to all available network interfaces
	 uint16 gamePort = 7777; // ER Server Port
	 uint16 queryPort = 27016; // Master Server Update Port
	 EServerMode serverMode = eServerModeAuthenticationAndSecure; // The server mode
	 const char* version = "1.0.0.0"; // Your game version
	
	 if (SteamGameServer_Init(ip, gamePort, queryPort, serverMode, version))
	 {
	 	UE_LOG(LogTemp, Log, TEXT("Steam game server initialized successfully"));
	
	 	// Initialize server settings
	 	// Todo: Make these variables stored in a config file
	 	SteamGameServer()->SetServerName("ER Test");
	 	SteamGameServer()->SetProduct("ER");
	 	SteamGameServer()->SetGameDescription("Senior Project");
	 	SteamGameServer()->SetModDir("spacewar");
	 	SteamGameServer()->SetMaxPlayerCount(MAX_PLAYERS_PER_SERVER);
	 	SteamGameServer()->SetGameTags("ERTest");
	 	
	 	FString MapName = GetMapName();
	 	UE_LOG(LogTemp, Warning, TEXT("Map name: %s"), *MapName);
	 	SteamGameServer()->SetMapName(TCHAR_TO_ANSI(*MapName));
	 	
	 	// Call SetDedicatedServer before LogOnAnonymous
	 	SteamGameServer()->SetDedicatedServer(true);
	 	SteamGameServer()->LogOnAnonymous();
	
	 	// Begin heartbeats to the Steam master server
		SteamGameServer()->SetAdvertiseServerActive(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize Steam game server"));
	}
	
	SteamNetworkingIPAddr serverAddr;
	serverAddr.Clear();
	serverAddr.SetIPv4(ip, gamePort); // Replace with your server IP and port
	ListenSocket = SteamGameServerNetworkingSockets()->CreateListenSocketIP(serverAddr, 0, nullptr);
	PollGroup = SteamGameServerNetworkingSockets()->CreatePollGroup();
	
	UE_LOG(LogTemp, Log, TEXT("Set Listen Socket and Poll Group"));

	/*IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
	OnlineSub->SetForceDedicated(true);
	if (OnlineSub)
	{
		IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionSettings = MakeShareable(new FOnlineSessionSettings());
			SessionSettings->bIsLANMatch = false;
			SessionSettings->bIsDedicated = true;
			SessionSettings->bUsesPresence = false;
			SessionSettings->NumPublicConnections = MAX_PLAYERS_PER_SERVER;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->Set(SETTING_MAPNAME, FString("er_downtown"), EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings->Set(TEXT("ServerName"), FString("Test"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			
			OnCreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			
			// Create the dedicated session
			SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);

			UE_LOG(LogTemp, Warning, TEXT("Steam Dedicated server successfully started"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystemSteam is not available"));
	}*/
}

void AServerGameMode::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the SessionComplete delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
			if (bWasSuccessful)
			{
				// Set the StartSession delegate handle
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				// Our StartSessionComplete delegate should get called after this
				Sessions->StartSession(SessionName);
			}
		}
		
	}
}

void AServerGameMode::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the delegate, since we are done with this call
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	if (bWasSuccessful)
	{
		//UGameplayStatics::OpenLevel(GetWorld(), "NewMap", true, "listen");
	}
}

// void AServerGameMode::OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* Callback)
// {
// 	check(Callback != nullptr);
// 	
// 	HSteamNetConnection hConn = Callback->m_hConn;
// 	SteamNetConnectionInfo_t info = Callback->m_info;
// 	ESteamNetworkingConnectionState OldState = Callback->m_eOldState;
// }
//
// void AServerGameMode::RemovePlayerFromServer(uint32 PlayerIndex, EDisconnectReason Reason)
//  {
// 	SteamGameServerNetworkingSockets()->CloseConnection( ClientData[PlayerIndex].m_hConn, Reason, nullptr, false);
//
// 	#ifdef USE_GS_AUTH_API
// 		// Tell the GS the user is leaving the server
// 		SteamGameServer()->EndAuthSession( ClientData[PlayerIndex].m_SteamIDUser );
// 	#endif
//  }
//
// bool AServerGameMode::SendDataToClient( uint32 PlayerIndex, char *Data, uint32 SizeOfData )
// {
// 	// Validate index
// 	if ( PlayerIndex >= MAX_PLAYERS_PER_SERVER )
// 		return false;
//
// 	int64 messageOut;
// 	if (!SteamGameServerNetworkingSockets()->SendMessageToConnection(ClientData[PlayerIndex].m_hConn, Data, SizeOfData, k_nSteamNetworkingSend_Unreliable, &messageOut))
// 	{
// 		UE_LOG(LogTemp, Log, TEXT("Failed sending data to client."));
// 		return false;
// 	}
// 	return true;
// }
//
// bool AServerGameMode::SendDataToPendingClient( uint32 PlayerIndex, char *Data, uint32 SizeOfData )
// {
// 	// Validate index
// 	if ( PlayerIndex >= MAX_PLAYERS_PER_SERVER )
// 		return false;
//
// 	int64 messageOut;
// 	if (!SteamGameServerNetworkingSockets()->SendMessageToConnection(PendingClientData[PlayerIndex].m_hConn, Data, SizeOfData, k_nSteamNetworkingSend_Unreliable, &messageOut))
// 	{
// 		UE_LOG(LogTemp, Log, TEXT("Failed sending data to client."));
// 		return false;
// 	}
// 	return true;
// }
//
// void AServerGameMode::OnSteamServersConnected( SteamServersConnected_t *pLogonSuccess )
// {
// 	UE_LOG(LogTemp, Log, TEXT("Server connnected to Steam successfully."));
// 	bConnectedToSteam = true;
//
// 	// log on is not finished until OnPolicyResponse() is called
//
// 	// Tell Steam about our server details
// 	SendUpdatedServerDetailsToSteam();
// }
//
// void AServerGameMode::OnPolicyResponse( GSPolicyResponse_t *pPolicyResponse )
// {
// #ifdef USE_GS_AUTH_API
// 	// Check if we were able to go VAC secure or not
// 	if ( SteamGameServer()->BSecure() )
// 	{
// 		UE_LOG(LogTemp, Log, TEXT( "Server is VAC Secure!" ));
// 	}
// 	else
// 	{
// 		UE_LOG(LogTemp, Log, TEXT( "Server is not VAC Secure!" ));
// 	}
// #endif
// }
//
// void AServerGameMode::OnSteamServersDisconnected( SteamServersDisconnected_t *pLoggedOff )
// {
// 	bConnectedToSteam = false;
// 	UE_LOG(LogTemp, Log, TEXT("Server got disconnected from Steam."));
// }
//
// void AServerGameMode::OnSteamServersConnectFailure( SteamServerConnectFailure_t *pConnectFailure )
// {
// 	bConnectedToSteam = false;
// 	UE_LOG(LogTemp, Log, TEXT("Server failed to connect to Steam."));
// }
//
// void AServerGameMode::SendUpdatedServerDetailsToSteam()
// {
// 	//
// 	// Set state variables, relevant to any master server updates or client pings
// 	//
//
// 	// These server state variables may be changed at any time.  Note that there is no longer a mechanism
// 	// to send the player count.  The player count is maintained by steam and you should use the player
// 	// creation/authentication functions to maintain your player count.
// 	SteamGameServer()->SetMaxPlayerCount( 32 );
// 	SteamGameServer()->SetPasswordProtected( false );
// 	SteamGameServer()->SetServerName( "ER Test" );
// 	SteamGameServer()->SetBotPlayerCount( 0 ); // optional, defaults to zero
//
// 	FString MapName = GetMapName();
// 	UE_LOG(LogTemp, Warning, TEXT("Map name: %s"), *MapName);
// 	SteamGameServer()->SetMapName(TCHAR_TO_ANSI(*MapName));
// }
//
// void AServerGameMode::OnClientBeginAuthentication(CSteamID steamIDClient, HSteamNetConnection connectionID, void* pToken, uint32 uTokenLen)
// {
// 	// First, check this isn't a duplicate and we already have a user logged on from the same steamid
// 	for (uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
// 	{
// 		if (ClientData[i].m_hConn == connectionID)
// 		{
// 			// We already logged them on... (should maybe tell them again incase they don't know?)
// 			return;
// 		}
// 	}
//
// 	// Second, do we have room?
// 	uint32 nPendingOrActivePlayerCount = 0;
// 	for (uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
// 	{
// 		if (PendingClientData[i].m_bActive)
// 			++nPendingOrActivePlayerCount;
//
// 		if (ClientData[i].m_bActive)
// 			++nPendingOrActivePlayerCount;
// 	}
//
// 	// We are full (or will be if the pending players auth), deny new login
// 	if ( nPendingOrActivePlayerCount >= MAX_PLAYERS_PER_SERVER )
// 	{
// 		SteamGameServerNetworkingSockets()->CloseConnection(connectionID, k_EDRServerFull, "Server full", false);
// 	}
//
// 	// If we get here there is room, add the player as pending
// 	for (uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
// 	{
// 		if (!PendingClientData[i].m_bActive)
// 		{
// 			PendingClientData[i].m_ulTickCountLastData = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64());
// #ifdef USE_GS_AUTH_API
// 			// authenticate the user with the Steam back-end servers
// 			EBeginAuthSessionResult res = SteamGameServer()->BeginAuthSession(pToken, uTokenLen, steamIDClient);
// 			if (res != k_EBeginAuthSessionResultOK)
// 			{
// 				SteamGameServerNetworkingSockets()->CloseConnection(connectionID, k_EDRServerReject, "BeginAuthSession failed", false);
// 				break;
// 			}
//
// 			PendingClientData[i].m_SteamIDUser = steamIDClient;
// 			PendingClientData[i].m_bActive = true;
// 			PendingClientData[i].m_hConn = connectionID;
// 			break;
// #else
// 			PendingClientData[i].m_bActive = true;
// 			// we need to tell the server our Steam id in the non-auth case, so we stashed it in the login message, pull it back out
// 			PendingClientData[i].m_SteamIDUser = *(CSteamID*)pToken;
// 			PendingClientData[i].m_connection = connectionID;
// 			// You would typically do your own authentication method here and later call OnAuthCompleted
// 			// In this sample we just automatically auth anyone who connects
// 			OnAuthCompleted(true, i);
// 			break;
// #endif
// 		}
// 	}
// }
//
// void AServerGameMode::OnAuthCompleted( bool bAuthSuccessful, uint32 iPendingAuthIndex )
// {
// 	if ( !PendingClientData[iPendingAuthIndex].m_bActive )
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT( "Got auth completed callback for client who is not pending" ));
// 		return;
// 	}
//
// 	if ( !bAuthSuccessful )
// 	{
// #ifdef USE_GS_AUTH_API
// 		// Tell the GS the user is leaving the server
// 		SteamGameServer()->EndAuthSession( PendingClientData[iPendingAuthIndex].m_SteamIDUser );
// #endif
// 		// Send a deny for the client, and zero out the pending data
// 		MsgServerFailAuthentication_t msg;
// 		int64 outMessage;
// 		SteamGameServerNetworkingSockets()->SendMessageToConnection(PendingClientData[iPendingAuthIndex].m_hConn, &msg, sizeof(msg), k_nSteamNetworkingSend_Reliable, &outMessage);
// 		PendingClientData[iPendingAuthIndex] = ClientConnectionData();
// 		return;
// 	}
//
// 	bool bAddedOk = false;
// 	for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i ) 
// 	{
// 		if ( !ClientData[i].m_bActive )
// 		{
// 			// copy over the data from the pending array
// 			memcpy( &ClientData[i], &PendingClientData[iPendingAuthIndex], sizeof( ClientConnectionData ) );
// 			PendingClientData[iPendingAuthIndex] = ClientConnectionData();
// 			//ClientData[i].m_ulTickCountLastData = m_pGameEngine->GetGameTickCount();
//
// 			bAddedOk = true;
//
// 			break;
// 		}
// 	}
//
// 	// If we just successfully added the player, check if they are #2 so we can restart the round
//
// 	if (bAddedOk)
// 	{
// 		UE_LOG(LogTemp, Log, TEXT("Added player successfully, todo"));
// 	}
// 	
// 	/*if ( bAddedOk )
// 	{
// 		uint32 uPlayers = 0;
// 		for( uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i ) 
// 		{
// 			if ( ClientData[i].m_bActive )
// 				++uPlayers;
// 		}
//
// 		// If we just got the second player, immediately reset round as a draw.  This will prevent
// 		// the existing player getting a win, and it will cause a new round to start right off
// 		// so that the one player can't just float around not letting the new one get into the game.
// 		if ( uPlayers == 2 )
// 		{
// 			if ( m_eGameState != k_EServerWaitingForPlayers )
// 				SetGameState( k_EServerDraw );
// 		}
// 	}*/
// }
//
// void AServerGameMode::OnValidateAuthTicketResponse( ValidateAuthTicketResponse_t *pResponse )
// {
// 	if ( pResponse->m_eAuthSessionResponse == k_EAuthSessionResponseOK )
// 	{
// 		// This is the final approval, and means we should let the client play (find the pending auth by steamid)
// 		for ( uint32 i = 0; i<MAX_PLAYERS_PER_SERVER; ++i )
// 		{
// 			if ( !PendingClientData[i].m_bActive )
// 				continue;
// 			else if ( PendingClientData[i].m_SteamIDUser == pResponse->m_SteamID )
// 			{
// 				UE_LOG(LogTemp, Log, TEXT("Auth completed for user"));
// 				OnAuthCompleted( true, i );
// 				return;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		// Looks like we shouldn't let this user play, kick them
// 		for ( uint32 i = 0; i<MAX_PLAYERS_PER_SERVER; ++i )
// 		{
// 			if ( !PendingClientData[i].m_bActive )
// 				continue;
// 			else if ( PendingClientData[i].m_SteamIDUser == pResponse->m_SteamID )
// 			{
// 				UE_LOG(LogTemp, Log, TEXT("Auth failed for user"));
// 				OnAuthCompleted( false, i );
// 				return;
// 			}
// 		}
// 	}
// }
//
 FString AServerGameMode::GetMapName() const
 {
 	UWorld* World = GetWorld();
 	if (World)
 	{
		FString MapName = World->GetMapName();
 		// Remove any prefix like "/Game/Maps/" if needed
 		MapName.RemoveFromStart(World->StreamingLevelsPrefix);
 		return MapName;
 	}
 	return FString("Unknown");
 }

void AServerGameMode::ShutdownSteamServer()
{
	SteamGameServerNetworkingSockets()->CloseListenSocket(ListenSocket);
	SteamGameServerNetworkingSockets()->DestroyPollGroup(PollGroup);
	
	SteamGameServer()->LogOff();
	SteamGameServer_Shutdown();
	
	UE_LOG(LogTemp, Log, TEXT("Steam game server shut down"));
}
//
// void AServerGameMode::ReceiveNetworkData()
// {
// 	SteamNetworkingMessage_t* msgs[128];
// 	int numMessages = SteamGameServerNetworkingSockets()->ReceiveMessagesOnPollGroup(PollGroup, msgs, 128);
// 	for (int idxMsg = 0; idxMsg < numMessages; idxMsg++)
// 	{
// 		SteamNetworkingMessage_t* message = msgs[idxMsg];
// 		CSteamID steamIDRemote = message->m_identityPeer.GetSteamID();
// 		HSteamNetConnection connection = message->m_conn;
//
// 		if (message->GetSize() < sizeof(DWORD))
// 		{
// 			UE_LOG(LogTemp, Log, TEXT("Got garbage on server socket, too short"));
// 			message->Release();
// 			message = nullptr;
// 			continue;
// 		}
//
// 		EMessage eMsg = (EMessage)(*(DWORD*)message->GetData());
//
// 		switch (eMsg)
// 		{
// 		case k_EMsgClientBeginAuthentication:
// 		{
// 			if (message->GetSize() != sizeof(MsgClientBeginAuthentication_t))
// 			{
// 				UE_LOG(LogTemp, Log, TEXT("Bad connection attempt msg"));
// 				message->Release();
// 				message = nullptr;
// 				continue;
// 			}
// 			MsgClientBeginAuthentication_t* pMsg = (MsgClientBeginAuthentication_t*)message->GetData();
// #ifdef USE_GS_AUTH_API
// 			OnClientBeginAuthentication(steamIDRemote, connection, (void*)pMsg->GetTokenPtr(), pMsg->GetTokenLen());
// #else
// 			OnClientBeginAuthentication(connection, 0);
// #endif
// 		}
// 		break;
// 		case k_EMsgClientSendLocalUpdate:
// 		{
// 			if (message->GetSize() != sizeof(MsgClientSendLocalUpdate_t))
// 			{
// 				UE_LOG(LogTemp, Log, TEXT("Bad Client Update MSG"))
// 				message->Release();
// 				message = nullptr;
// 				continue;
// 			}
//
// 			// Find the connection that should exist for this users address
// 			bool bFound = false;
// 			for (uint32 i = 0; i < MAX_PLAYERS_PER_SERVER; ++i)
// 			{
// 				if (ClientData[i].m_hConn == connection)
// 				{
// 					bFound = true;
// 					break;
// 				}
// 			}
// 			if (!bFound)
// 				UE_LOG(LogTemp, Log, TEXT("Couldn't find matching client but received client update"))
// 		}
// 		break;
//
// 		case k_EMsgVoiceChatData:
// 		{
// 				UE_LOG(LogTemp, Log, TEXT("EMSG Voice Chat Data Reached"))
// 			break;
// 		}
// 		case k_EMsgP2PSendingTicket:
// 		{
// 			UE_LOG(LogTemp, Log, TEXT("P2P Sending Ticket Reached"))
// 		}
// 		break;
//
// 		default:
// 			char rgch[128];
// 			printf(rgch, "Invalid message %x\n", eMsg);
// 			rgch[sizeof(rgch) - 1] = 0;
// 			UE_LOG(LogTemp, Log, TEXT("Auth failed for user"));
// 		}
//
// 		message->Release();
// 		message = nullptr;
// 	}
// }
	
void AServerGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ReceiveNetworkData();
	//
	// // Creates a heartbeat to the steam master server and operates all callback functions on the server
	// SteamGameServer_RunCallbacks();
	//
	// //SendUpdatedServerDetailsToSteam();
	//
	// uint32 uPlayerCount = 0;
	// for( uint32 i=0; i < MAX_PLAYERS_PER_SERVER; ++i )
	// {
	// 	// If there is no ship, skip
	// 	if ( !ClientData[i].m_bActive )
	// 		continue;
	//
	// 	if ( FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64()) - ClientData[i].m_ulTickCountLastData > 5000 )
	// 	{
	// 		UE_LOG(LogTemp, Log, TEXT("Timing out player connection"));
	// 		RemovePlayerFromServer( i, k_EDRClientKicked );
	// 	}
	// 	else
	// 	{
	// 		++uPlayerCount;
	// 	}
	// }
	// PlayerCount = uPlayerCount;
}
