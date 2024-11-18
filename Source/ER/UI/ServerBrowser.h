// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ServerEntry.h"
#include "Blueprint/UserWidget.h"
#include "ServerList.h"
#include "SteamLibrary/Public/SteamServerWrapper.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
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
	void OnConnectButtonClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnRefreshButtonClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnServerEntryClicked(USteamServerWrapper* Server, bool isSelected);

	UFUNCTION()
	void UpdateServerList();

private:
	UPROPERTY()
	USteamServerWrapper* SelectedServer;

	// Set a button as active (change visual appearance)
	void SetButtonActive(UButton* Button);
	void SetButtonInactive(UButton* Button);
	
protected:
	UPROPERTY(meta = (BindWidget))
	UButton* ConnectButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;
	
	UPROPERTY(meta = (BindWidget))
	UServerList* ServerList;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* FullBool;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* PlayersBool;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* PasswordBool;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ServerTextBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* MapTextBox;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* LatencyBox;

	UPROPERTY(meta = (BindWidget))
	UButton* InternetButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FavoritesButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AddFavoritesButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RemoveFavoritesButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* ServerNameSort;
	
	UPROPERTY(meta = (BindWidget))
	UButton* MapNameSort;
	
	UPROPERTY(meta = (BindWidget))
	UButton* PlayersSort;
	
	UPROPERTY(meta = (BindWidget))
	UButton* LatencySort;
	
	UFUNCTION()
	void OnServerTextChanged(const FText& Text);

	UFUNCTION()
	void OnMapTextChanged(const FText& Text);

	UFUNCTION()
	void OnLatencySelected(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void FilterServersByLatency(int32 LatencyThreshold);

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	bool bIsServerTextEntered = false;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	bool bIsMapTextEntered = false;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	bool bFavoriteSelected = false;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	bool bIsAscendingOrder = true;
	
	UFUNCTION()
	void OnInternetButtonClicked();
	
	UFUNCTION()
	void OnFavoritesButtonClicked();

	UFUNCTION()
	void OnAddFavoritesButtonClicked();
	
	UFUNCTION()
	void OnRemoveFavoritesButtonClicked();

	UFUNCTION()
	void OnServerNameSortClicked();

	UFUNCTION()
	void SortServersByName(bool bAscending);

	UFUNCTION()
	void SortServersByMap(bool bAscending);

	UFUNCTION()
	void SortServersByPlayers(bool bAscending);

	UFUNCTION()
	void SortServersByLatency(bool bAscending);

	UFUNCTION()
	void OnMapNameSortClicked();

	UFUNCTION()
	void OnPlayersSortClicked();

	UFUNCTION()
	void OnLatencySortClicked();
};
