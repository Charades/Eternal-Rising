// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HAL/RunnableThread.h"
#include "ClientGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ER_API UClientGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
	UFUNCTION()
	virtual void BeginLoadingScreen(const FString& MapName);
	
	UFUNCTION()
	virtual void EndLoadingScreen(UWorld* InLoadedWorld);

	UFUNCTION()
	virtual void ReturnToMainMenu() override;
};
