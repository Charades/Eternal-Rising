// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerBrowser.h"
#include "ServerEntry.h"
#include "ClientNetworkSubsystem.h"

void UServerBrowser::NativeConstruct()
{
	Super::NativeConstruct();

	ServerList = Cast<UServerList>(GetWidgetFromName(TEXT("ServerList")));
	RefreshButton = Cast<UButton>(GetWidgetFromName(TEXT("RefreshButton")));
	
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UServerBrowser::OnRefreshButtonClicked);
	}

	if (ServerList)
	{
		ServerList->OnServerListItemSelectionChanged.AddUObject(this, &UServerBrowser::OnServerEntryClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			ClientNetworkSubsystem->OnServerListUpdated.AddDynamic(this, &UServerBrowser::UpdateServerList);
		}
	}
}

void UServerBrowser::OnServerEntryClicked(USteamServerWrapper* Server, bool isSelected)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Server entry selected"));
	
	if (Server)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Server->ConnectionIP());
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Connection Info is valid"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Connection Info is invalid"));
	}
}

void UServerBrowser::OnRefreshButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Refresh button clicked!"));
	}

	RefreshServerList();
}

void UServerBrowser::PopulateServerList(const TArray<USteamServerWrapper*>& Servers)
{
	if (!ServerList) return;
	
	ServerList->ClearListItems();
	ServerList->SetListItems(Servers);
}

void UServerBrowser::UpdateServerList()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
			if (ClientNetworkSubsystem)
			{
				PopulateServerList(ClientNetworkSubsystem->GameServerList);
			}
		}
	});
}


void UServerBrowser::RefreshServerList()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			ClientNetworkSubsystem->RequestServerList();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ClientNetworkSubsystem is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance is null"));
	}
}
