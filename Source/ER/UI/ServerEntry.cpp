// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerEntry.h"

#include "ServerList.h"
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

	USteamServerWrapper* ServerData = Cast<USteamServerWrapper>(ListItemObject);
	if (!ServerData) return;

	// Get the owning ListView
	UListView* OwningListView = Cast<UListView>(GetOwningListView());
	if (!OwningListView) return;

	// Determine the index of this item in the ListView
	const int32 Index = OwningListView->GetIndexForItem(ListItemObject);

	if(IsListItemSelected())
	{
		SetBorderColor(FLinearColor(0.670588f, 0.313726f, 0.0f, 1.0f)); // Example hover color
	}

	// Apply alternating row colors based on the index (even/odd)
	UBorder* BackgroundBorder = Cast<UBorder>(GetWidgetFromName(TEXT("EntryBorder")));
	if (BackgroundBorder)
	{
		if (Index % 2 == 0)
		{
			// Even index: Dark color
			BackgroundBorder->SetBrushColor(FLinearColor(0.015, 0.015, 0.015, 1.0f));
		}
		else
		{
			// Odd index: Light color
			BackgroundBorder->SetBrushColor(FLinearColor(0.025, 0.025, 0.025, 1.0f));
		}
	}
}

FString UServerEntry::GetConnectionInfo() const
{
	return *ServerID;
}

void UServerEntry::SetBorderColor(const FLinearColor& NewColor)
{
	if (EntryBorder)
	{
		EntryBorder->SetBrushColor(NewColor);
		UE_LOG(LogTemp, Log, TEXT("Border color updated."));
	}
}

void UServerEntry::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	// Change the border color on hover
	if(!IsListItemSelected())
	{
		SetBorderColor(FLinearColor(0.045f, 0.045f, 0.045f, 1.0f)); // Example hover color
	}
}

void UServerEntry::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	
	UListView* OwningListView = Cast<UListView>(GetOwningListView());
	if (!OwningListView) return;
	
	// Reset the border color when the mouse leaves
	const int32 Index = OwningListView  ? OwningListView->GetIndexForItem(GetListItem()) : INDEX_NONE;
	if (Index != INDEX_NONE && !IsListItemSelected())
	{
		// Alternate color based on index
		if (Index % 2 == 0)
		{
			SetBorderColor(FLinearColor(0.015f, 0.015f, 0.015f, 1.0f)); // Even index color
		}
		else
		{
			SetBorderColor(FLinearColor(0.025f, 0.025f, 0.025f, 1.0f)); // Odd index color
		}
	}
}