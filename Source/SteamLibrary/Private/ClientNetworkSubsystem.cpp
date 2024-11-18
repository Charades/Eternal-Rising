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

void UClientNetworkSubsystem::RequestInternetServerList(const TArray<FString>& FilterKeys, const TArray<FString>& FilterValues, bool bFilterPassword)
{
	if (bRequestingServers)
	{
		SteamMatchmakingServers()->CancelQuery(ServerRequestHandle);
		return;
	}
	
	if (ServerRequestHandle)
	{
		SteamMatchmakingServers()->ReleaseRequest(ServerRequestHandle);
		ServerRequestHandle = nullptr;
	}

	bHasPassword = bFilterPassword;
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Refreshing internet servers"));

	bRequestingServers = true;
	nServers = 0;
	GameServerList.Empty();

	ServerListResponse = new FSteamServerCallback(this);

	if (FilterKeys.Num() != FilterValues.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("FilterKeys and FilterValues array sizes do not match."));
		return;
	}

	// Create an array of filters
	TArray<MatchMakingKeyValuePair_t> Filters;
	for (int32 i = 0; i < FilterKeys.Num(); ++i)
	{
		MatchMakingKeyValuePair_t Filter;
		strncpy_s(Filter.m_szKey, TCHAR_TO_ANSI(*FilterKeys[i]), sizeof(Filter.m_szKey));
		
		if (FilterValues[i].IsEmpty())
		{
			Filter.m_szValue[0] = '\0'; // Set value to an empty string for boolean filters
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Filter %d: Key = %s, Value = (empty)"), i, *FilterKeys[i]));
		}
		else
		{
			strncpy_s(Filter.m_szValue, TCHAR_TO_ANSI(*FilterValues[i]), sizeof(Filter.m_szValue));
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Filter %d: Key = %s, Value = %s"), i, *FilterKeys[i], *FilterValues[i]));
		}
		
		Filters.Add(Filter);
	}
	
	MatchMakingKeyValuePair_t* pFilters = Filters.GetData();
	
	ServerRequestHandle = SteamMatchmakingServers()->RequestInternetServerList(
		480, // Your Steam App ID
		&pFilters, // Filters
		Filters.Num(), // Number of filters
		ServerListResponse // Request callback
	);
}

void UClientNetworkSubsystem::RequestFavoriteServerList(const TArray<FString>& FilterKeys, const TArray<FString>& FilterValues, bool bFilterPassword)
{
	if (bRequestingServers)
	{
		SteamMatchmakingServers()->CancelQuery(ServerRequestHandle);
		return;
	}
	
	if (ServerRequestHandle)
	{
		SteamMatchmakingServers()->ReleaseRequest(ServerRequestHandle);
		ServerRequestHandle = nullptr;
	}

	bHasPassword = bFilterPassword;
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Refreshing internet servers"));

	bRequestingServers = true;
	nServers = 0;
	GameServerList.Empty();

	ServerListResponse = new FSteamServerCallback(this);

	if (FilterKeys.Num() != FilterValues.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("FilterKeys and FilterValues array sizes do not match."));
		return;
	}

	// Create an array of filters
	TArray<MatchMakingKeyValuePair_t> Filters;
	for (int32 i = 0; i < FilterKeys.Num(); ++i)
	{
		MatchMakingKeyValuePair_t Filter;
		strncpy_s(Filter.m_szKey, TCHAR_TO_ANSI(*FilterKeys[i]), sizeof(Filter.m_szKey));
		
		if (FilterValues[i].IsEmpty())
		{
			Filter.m_szValue[0] = '\0'; // Set value to an empty string for boolean filters
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Filter %d: Key = %s, Value = (empty)"), i, *FilterKeys[i]));
		}
		else
		{
			strncpy_s(Filter.m_szValue, TCHAR_TO_ANSI(*FilterValues[i]), sizeof(Filter.m_szValue));
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::Printf(TEXT("Filter %d: Key = %s, Value = %s"), i, *FilterKeys[i], *FilterValues[i]));
		}
		
		Filters.Add(Filter);
	}
	
	MatchMakingKeyValuePair_t* pFilters = Filters.GetData();
	
	ServerRequestHandle = SteamMatchmakingServers()->RequestFavoritesServerList(
		480, // Your Steam App ID
		&pFilters, // Filters
		Filters.Num(), // Number of filters
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
				if (Server->m_bPassword && bHasPassword)
				{
					// Initialize the new server object and add it to the list
					NewServer->Initialize(Server);
					GameServerList.Add(NewServer);
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, GameServerList.Last()->GetName());
					nServers++;
				}
				else if (!bHasPassword)
				{
					// Initialize the new server object and add it to the list
					NewServer->Initialize(Server);
					GameServerList.Add(NewServer);
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, GameServerList.Last()->GetName());
					nServers++;
				}
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
