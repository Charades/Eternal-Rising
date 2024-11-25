// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowFieldWorld.h"
#include "Components/ActorComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "FlowFieldMovement.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ER_API UFlowFieldMovement : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFlowFieldMovement();

	UPROPERTY()
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;

	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	UPROPERTY()
	TObjectPtr<AFlowFieldWorld> FlowFieldActor;

	UPROPERTY()
	TMap<FVector2D, FVector> DirectionMap;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FVector GoalPosition;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	float GoalAcceptanceDist = 50.0f;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FVector MoveDirection;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	bool Move;

	UFUNCTION()
	void BeginMovement(TMap<FVector2D, FVector>& InDirectionMap, FVector& InGoalPosition);

	UFUNCTION()
	FVector2D FindCurrentCell(FVector InPawnWorldPos);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
