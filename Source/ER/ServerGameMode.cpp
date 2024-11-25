// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerGameMode.h"

#include "GameFramework/PlayerState.h"
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
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
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
	}
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
}

void AServerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Register the player with the session
	RegisterPlayer(NewPlayer);
}

void AServerGameMode::Logout(AController* Exiting)
{
	// Unregister the player from the session
	UnregisterPlayer(Exiting);

	Super::Logout(Exiting);
}


void AServerGameMode::RegisterPlayer(APlayerController* NewPlayer)
{
	IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
	if (!Sessions.IsValid() || !NewPlayer)
	{
		return;
	}

	// Get PlayerState and check if it's valid
	APlayerState* PlayerState = NewPlayer->PlayerState;
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is invalid."));
		return;
	}

	FUniqueNetIdRepl UniqueNetIdRepl = PlayerState->GetUniqueId();
	if (!UniqueNetIdRepl.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player UniqueNetId is invalid."));
		return;
	}
	
	if (UniqueNetIdRepl.IsValid())
	{
		Sessions->RegisterPlayer(FName("GameSession"), *UniqueNetIdRepl.GetUniqueNetId(), false);
		UE_LOG(LogTemp, Log, TEXT("Player registered: %s"), *UniqueNetIdRepl->ToString());
	}
}

void AServerGameMode::UnregisterPlayer(AController* Exiting)
{
	if (!Exiting)
	{
		UE_LOG(LogTemp, Warning, TEXT("Exiting controller is null."));
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Exiting);
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Exiting controller is not a player controller."));
		return;
	}

	IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
	if (!Sessions.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Online session interface is invalid."));
		return;
	}

	// Get PlayerState and check if it's valid
	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is invalid."));
		return;
	}

	// Get the unique player ID from PlayerState
	FUniqueNetIdRepl UniqueNetIdRepl = PlayerState->GetUniqueId();
	if (!UniqueNetIdRepl.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player UniqueNetId is invalid."));
		return;
	}

	// Unregister the player
	bool bUnregisterSuccess = Sessions->UnregisterPlayer(FName("GameSession"), *UniqueNetIdRepl.GetUniqueNetId());
	if (bUnregisterSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Player unregistered successfully: %s"), *UniqueNetIdRepl->ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to unregister player: %s"), *UniqueNetIdRepl->ToString());
	}
}

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
	UE_LOG(LogTemp, Log, TEXT("Steam game server shut down"));
}

void AServerGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
