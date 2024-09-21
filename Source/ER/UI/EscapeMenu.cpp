// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapeMenu.h"
#include "Components/Button.h"

UEscapeMenu::UEscapeMenu(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UServerBrowser> ServerBrowserWidgetFinder(
	TEXT("/Game/Blueprints/UI/ServerBrowser"));
	if (ServerBrowserWidgetFinder.Succeeded())
	{
		ServerBrowserWidget = ServerBrowserWidgetFinder.Class;
		UE_LOG(LogTemp, Log, TEXT("Found Server Browser Widget!"));
	}
}

void UEscapeMenu::NativeConstruct()
{
	Super::NativeConstruct();

	ServerBrowserButton = Cast<UButton>(GetWidgetFromName(TEXT("ServerBrowserButton")));
	ExitGameButton = Cast<UButton>(GetWidgetFromName(TEXT("ExitGameButton")));
	
	if (ServerBrowserButton)
	{
		ServerBrowserButton->OnClicked.AddDynamic(this, &UEscapeMenu::OnServerBrowserButtonClicked);
	}
	
	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.AddDynamic(this, &UEscapeMenu::OnExitGameButtonClicked);
	}
}

void UEscapeMenu::RemoveFromParent()
{
	if (ServerBrowser && ServerBrowser->IsInViewport())
	{
		ServerBrowser->RemoveFromParent();
		ServerBrowser = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Server Browser hidden."));
	}
	
	Super::RemoveFromParent();
}

void UEscapeMenu::OnServerBrowserButtonClicked()
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

void UEscapeMenu::OnExitGameButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;

	if (PlayerController)
	{
		UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, true);
	}
}


