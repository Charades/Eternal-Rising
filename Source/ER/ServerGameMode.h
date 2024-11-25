// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#define USE_GS_AUTH_API

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemSteam.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
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
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void Tick(float DeltaSeconds) override;
	void RegisterPlayer(APlayerController* NewPlayer);
	void UnregisterPlayer(AController* Exiting);

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
	
	bool bConnectedToSteam;
};
