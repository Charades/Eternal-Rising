// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerList.h"

void UServerList::OnSelectionChangedInternal(UObject* FirstSelectedItem)
{
	Super::OnSelectionChangedInternal(FirstSelectedItem);

	if (!FirstSelectedItem)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No item selected."));
		return;
	}
	
	USteamServerWrapper* Entry = Cast<USteamServerWrapper>(FirstSelectedItem);

	if (Entry)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Server entry cast successful."));
		OnServerListItemSelectionChanged.Broadcast(Entry, true);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Server entry cast failed."));
		OnServerListItemSelectionChanged.Broadcast(nullptr, false);
	}
}
