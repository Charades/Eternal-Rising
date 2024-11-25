// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientGameInstance.h"
#include "MoviePlayer.h"

void UClientGameInstance::Init()
{
    Super::Init();
    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UClientGameInstance::BeginLoadingScreen);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UClientGameInstance::EndLoadingScreen);
}

void UClientGameInstance::BeginLoadingScreen(const FString& InMapName)
{
	if (!IsRunningDedicatedServer())
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.bWaitForManualStop = false;
		LoadingScreen.bAllowEngineTick = true;
		LoadingScreen.MinimumLoadingScreenDisplayTime = -1;

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		GetMoviePlayer()->PlayMovie();
	}
}

void UClientGameInstance::ReturnToMainMenu()
{
	Super::ReturnToMainMenu();
	
	if (!IsRunningDedicatedServer())
	{
		APlayerController* PlayerController = GetFirstLocalPlayerController();
		if (PlayerController)
		{
			// Travel back to the main menu map (e.g., "MainMenuMap")
			PlayerController->ClientTravel("/Content/Maps/ClientMainMenu", TRAVEL_Absolute);
		}
	}
}

void UClientGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
	if (GetMoviePlayer()->IsMovieCurrentlyPlaying())
	{
		GetMoviePlayer()->StopMovie();
	}
}