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

void AServerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Register the player with the session
	RegisterPlayer(NewPlayer);
}

void AServerGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// Unregister the player from the session
	UnregisterPlayer(Exiting);
}

void AServerGameMode::RegisterPlayer(APlayerController* NewPlayer)
{
	IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
	if (!Sessions.IsValid() || !NewPlayer)
	{
		return;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IOnlineSubsystem::Get()->GetIdentityInterface()->GetUniquePlayerId(0);
	if (PlayerId.IsValid())
	{
		Sessions->RegisterPlayer(FName("GameSession"), *PlayerId, false);
		UE_LOG(LogTemp, Log, TEXT("Player registered: %s"), *PlayerId->ToString());
	}
}

void AServerGameMode::UnregisterPlayer(AController* Exiting)
{
	IOnlineSessionPtr Sessions = IOnlineSubsystem::Get()->GetSessionInterface();
	if (!Sessions.IsValid() || !Exiting)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Exiting);
	if (!PlayerController)
	{
		return;
	}

	TSharedPtr<const FUniqueNetId> PlayerId = IOnlineSubsystem::Get()->GetIdentityInterface()->GetUniquePlayerId(0);
	if (PlayerId.IsValid())
	{
		Sessions->UnregisterPlayer(FName("GameSession"), *PlayerId);
		UE_LOG(LogTemp, Log, TEXT("Player unregistered: %s"), *PlayerId->ToString());
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
