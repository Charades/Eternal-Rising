// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerEntry.h"
#include "Components/TextBlock.h"
#include "SteamServerWrapper.h"

void UServerEntry::NativeConstruct()
{
	Super::NativeConstruct();
}

void UServerEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	USteamServerWrapper* Server = Cast<USteamServerWrapper>(ListItemObject);
	ServerNameText->SetText(FText::FromString(Server->GetName()));
	MapNameText->SetText(FText::FromString(Server->GetMap()));
	PlayerCountText->SetText(FText::FromString(Server->GetPlayerCount()));
	ServerPingText->SetText(FText::AsNumber(Server->GetPing()));
	ServerID = Server->GetSteamID();
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Stored ID: %s"), *ServerID));
}

FString UServerEntry::GetConnectionInfo() const
{
	return *ServerID;
}