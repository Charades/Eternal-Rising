// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputSubsystems.h"
#include "InputData.h"
#include "UI/MarqueeSelectionWidget.h"
#include "ClientPlayerController.h"
#include "FlowFieldMovement.h"
#include "FlowFieldWorld.h"
#include "DirectorPawn.generated.h"

UCLASS()
class ER_API ADirectorPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ADirectorPawn();

	UPROPERTY()
	TObjectPtr<AFlowFieldWorld> FlowFieldActor;

	UPROPERTY()
	UMarqueeSelectionWidget* SelectionWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MarqueeWidgetClass;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputData* InputData;

	UPROPERTY()
	TArray<APawn*> SelectedPawns;

	UPROPERTY()
	TArray<UFlowFieldMovement*> PawnMovements;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void StartMarqueeSelection();
	void EndMarqueeSelection();
	void MoveToLocation();
	
	bool bIsSelecting = false;
	bool FoundStartPosition = false;
	FVector2D InitialMousePosition;
	FVector StartPosition;

	void PerformSelection(const FVector2D& TopLeft, const FVector2D& BottomRight);
};
