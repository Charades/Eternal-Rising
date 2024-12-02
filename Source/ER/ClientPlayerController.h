// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "ClientNetworkSubsystem.h"
#include "UI/EscapeMenu.h"
#include "ClientPlayerController.generated.h"

class UInputMappingContext;
class UInputData;

UCLASS()
class ER_API AClientPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AClientPlayerController();

	UFUNCTION(Exec)
	void ConnectToServer(const FString& ServerSteamID);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestSpawnHorde(FVector HordeSpawnLocation, float Radius, int32 NumEntities);
	
	UInputMappingContext* GetDefaultMappingContext() const { return DefaultMappingContext; }
	UInputData* GetInputData() const { return InputData; }

	void SpawnActors();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnShowEscapeMenu(const FInputActionValue& Value);
	void LeftMouseClick(const FInputActionValue& Value);
	void RightMouseClick(const FInputActionValue& Value);


private:
	HSteamNetConnection ServerConnection;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* MenuMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputData* InputData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UEscapeMenu> EscapeMenuWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> LoadingScreenWidget;
	
	UPROPERTY()
	UEscapeMenu* EscapeMenu;

	UPROPERTY()
	UUserWidget* LoadingScreen;
};
