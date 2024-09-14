// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ServerEntry.h"
#include "Blueprint/UserWidget.h"
#include "ServerList.h"
#include "SteamLibrary/Public/SteamServerWrapper.h"
#include "Components/Button.h"
#include "ServerBrowser.generated.h"

UCLASS()
class ER_API UServerBrowser : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void PopulateServerList(const TArray<USteamServerWrapper*>& Servers);
	void RefreshServerList();
	
	UFUNCTION(BlueprintCallable)
	void OnRefreshButtonClicked();

	UFUNCTION(BlueprintCallable)
	void OnServerEntryClicked(USteamServerWrapper* Server, bool isSelected);

	UFUNCTION()
	void UpdateServerList();

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;
	
	UPROPERTY(meta = (BindWidget))
	UServerList* ServerList;
};
