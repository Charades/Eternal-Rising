// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ServerBrowser.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MainMenu.generated.h"

UCLASS()
class ER_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UButton* ServerBrowserButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitGameButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* SettingsMenuButton;
	

	UMainMenu(const FObjectInitializer& ObjectInitializer);

	UFUNCTION()
	void NativeConstruct() override;
	void RemoveFromParent() override;
	
	UFUNCTION()
	void OnServerBrowserButtonClicked();

	UFUNCTION()
	void OnSettingsMenuButtonClicked();

	UFUNCTION()
	void OnExitGameButtonClicked();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UServerBrowser> ServerBrowserWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> SettingsMenuWidget;

	UPROPERTY()
	UServerBrowser* ServerBrowser;

	UPROPERTY()
	UUserWidget* SettingsMenu;
};
