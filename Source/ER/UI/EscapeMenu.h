// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ServerBrowser.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EscapeMenu.generated.h"

UCLASS()
class ER_API UEscapeMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UButton* ServerBrowserButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitGameButton;

	UEscapeMenu(const FObjectInitializer& ObjectInitializer);
	
	void NativeConstruct() override;
	void RemoveFromParent() override;
	
	UFUNCTION()
	void OnServerBrowserButtonClicked();

	UFUNCTION()
	void OnExitGameButtonClicked();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UServerBrowser> ServerBrowserWidget;

	UPROPERTY()
	UServerBrowser* ServerBrowser;
};
