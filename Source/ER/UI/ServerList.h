// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "SteamLibrary/Public/SteamServerWrapper.h"
#include "ServerList.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnServerListItemSelectionChanged, USteamServerWrapper*, bool);

UCLASS(meta = (EntryInterface = UserObjectListEntry, EntryClass = UServerEntry))
class ER_API UServerList : public UListView
{
	GENERATED_BODY()

public:
	FOnServerListItemSelectionChanged OnServerListItemSelectionChanged;
	
private:
	virtual void OnSelectionChangedInternal(UObject* FirstSelectedItem) override;
};
