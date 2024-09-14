// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientNetworkSubsystem.h"
#include "ER/UI/ServerBrowser.h"
#include "Async/Async.h"

void UClientNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("ClientNetworkSubsystem initialized!"));
	
	if (!SteamAPI_Init())
	{
		UE_LOG(LogTemp, Warning, TEXT("Steam API failed to initialize"));
	}
}

void UClientNetworkSubsystem::Deinitialize()
{
	if (ServerRequestHandle)
	{
		SteamMatchmakingServers()->ReleaseRequest(ServerRequestHandle);
		ServerRequestHandle = nullptr;
	}
	
	if (ServerListResponse)
	{
		delete ServerListResponse;
		ServerListResponse = nullptr;
	}
	
	SteamAPI_Shutdown();
	Super::Deinitialize();
}


void UClientNetworkSubsystem::RequestServerList()
{
	if (bRequestingServers)
	{
		return;
	}
	
	if (ServerRequestHandle)
	{
		SteamMatchmakingServers()->ReleaseRequest(ServerRequestHandle);
		ServerRequestHandle = nullptr;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Refreshing internet servers"));

	bRequestingServers = true;
	nServers = 0;
	GameServerList.Empty();

	ServerListResponse = new FSteamServerCallback(this);

	MatchMakingKeyValuePair_t pFilters[2];
	MatchMakingKeyValuePair_t *pFilter = pFilters;
	
	strncpy_s(pFilters[0].m_szKey, "gamedir", sizeof(pFilters[0].m_szKey));
	strncpy_s(pFilters[0].m_szValue, "spacewar", sizeof(pFilters[0].m_szValue));

	strncpy_s(pFilters[1].m_szKey, "secure", sizeof(pFilters[1].m_szKey));
	strncpy_s(pFilters[1].m_szValue, "1", sizeof(pFilters[1].m_szValue));
	
	ServerRequestHandle = SteamMatchmakingServers()->RequestInternetServerList(
		480, // Your Steam App ID
		&pFilter, // Filters
		2, // Number of filters
		ServerListResponse // Request callback
	);
}

void UClientNetworkSubsystem::OnServerResponded(HServerListRequest Request, int Index)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("OnServerResponded"));
	gameserveritem_t* Server = SteamMatchmakingServers()->GetServerDetails(Request, Index);
	if (Server)
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("App ID: %hs"), Server->m_szGameDescription));
	{
		if (Server->m_nAppID == 480)
		{
			USteamServerWrapper* NewServer = NewObject<USteamServerWrapper>(this);
			if (NewServer)
			{
				// Initialize the new server object and add it to the list
				NewServer->Initialize(Server);
				GameServerList.Add(NewServer);
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, GameServerList.Last()->GetName());
				nServers++;
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to create new server object"));
			}
		}
	}
	
	NotifyServerListUpdated();
}

void UClientNetworkSubsystem::NotifyServerListUpdated()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
    {
        OnServerListUpdated.Broadcast();
    });
}

void UClientNetworkSubsystem::OnServerFailedToRespond(HServerListRequest Request, int Index)
{
	// Not really needed at this point
}

void UClientNetworkSubsystem::OnServerRefreshComplete(HServerListRequest HRequest, EMatchMakingServerResponse Response)
{
	bRequestingServers = false;
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Server list refresh complete"));
}
