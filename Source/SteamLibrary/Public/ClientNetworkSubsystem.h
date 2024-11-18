// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "steam/steam_api.h"
#include "SteamLibrary/Public/SteamServerCallback.h"
#include "SteamLibrary/Public/SteamServerWrapper.h"
#include "ClientNetworkSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnServerListUpdated);

UCLASS()
class STEAMLIBRARY_API UClientNetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnServerListUpdated OnServerListUpdated;
	void OnServerFailedToRespond(HServerListRequest Request, int Index);
	void OnServerRefreshComplete(HServerListRequest HRequest, EMatchMakingServerResponse Response);
	void RequestInternetServerList(const TArray<FString>& FilterKeys, const TArray<FString>& FilterValues, bool bFilterPassword);
	void RequestFavoriteServerList(const TArray<FString>& FilterKeys, const TArray<FString>& FilterValues, bool bFilterPassword);
	void OnServerResponded(HServerListRequest Request, int Index);
	
	UPROPERTY(BlueprintReadOnly, Category = "Game Server")
	TArray<USteamServerWrapper*> GameServerList;
	
private:
	HServerListRequest ServerRequestHandle;
	FSteamServerCallback* ServerListResponse;
	bool bRequestingServers;
	bool bHasPassword;
	int nServers;

protected:
	void NotifyServerListUpdated();
};
