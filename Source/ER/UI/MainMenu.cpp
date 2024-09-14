﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/Button.h"

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer),
                                                                   ServerBrowserButton(nullptr),
                                                                   ExitGameButton(nullptr),
                                                                   ServerBrowser(nullptr)
{
	static ConstructorHelpers::FClassFinder<UServerBrowser> ServerBrowserWidgetFinder(
		TEXT("/Game/GameContent/Blueprints/ServerBrowser"));
	if (ServerBrowserWidgetFinder.Succeeded())
	{
		ServerBrowserWidget = ServerBrowserWidgetFinder.Class;
		UE_LOG(LogTemp, Log, TEXT("Found Server Browser Widget!"));
	}
}

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	ServerBrowserButton = Cast<UButton>(GetWidgetFromName(TEXT("ServerBrowserButton")));
	ExitGameButton = Cast<UButton>(GetWidgetFromName(TEXT("ExitGameButton")));
	
	if (ServerBrowserButton)
	{
		ServerBrowserButton->OnClicked.AddDynamic(this, &UMainMenu::OnServerBrowserButtonClicked);
	}
	
	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.AddDynamic(this, &UMainMenu::OnExitGameButtonClicked);
	}
}

void UMainMenu::RemoveFromParent()
{
	if (ServerBrowser && ServerBrowser->IsInViewport())
	{
		ServerBrowser->RemoveFromParent();
		ServerBrowser = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Server Browser hidden."));
	}
	
	Super::RemoveFromParent();
}

void UMainMenu::OnServerBrowserButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Server button clicked!"));
	}

	if (ServerBrowserWidget)
	{
		if (!ServerBrowser)
		{
			ServerBrowser = CreateWidget<UServerBrowser>(this, ServerBrowserWidget);
		}

		if (ServerBrowser)
		{
			if (ServerBrowser->IsInViewport())
			{
				ServerBrowser->RemoveFromParent();
				ServerBrowser = nullptr;
				UE_LOG(LogTemp, Log, TEXT("Server Browser hidden."));
			}
			else
			{
				ServerBrowser->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("Server Browser displayed."));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerBrowserWidget is null."));
	}
}

void UMainMenu::OnExitGameButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;

	if (PlayerController)
	{
		UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, true);
	}
}


