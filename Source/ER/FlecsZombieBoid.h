// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FlecsZombieBoid.generated.h"

class AFlecsZombieHorde;
class AFlecsZombieStimulus;

UCLASS(BlueprintType, Blueprintable)
class ER_API UFlecsZombieBoid : public UObject
{
	GENERATED_BODY()

public:
	UFlecsZombieBoid();

	void ResetComponents();

	void Init(const FVector& Location, const FRotator& Rotation, int32 MeshInstanceIndex);

	void Update(float DeltaSeconds, AFlecsZombieHorde* Agent);

protected:
	// Steering Calculations
	void ComputeMovementVector(AFlecsZombieHorde* HordeAgent);

	// Component Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector TargetMoveVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Steering Behavior Component")
	FVector CurrentMoveVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector AlignmentVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector CohesionVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector SeparationVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector NegativeStimuliVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector PositiveStimuliVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	FVector CollisionAvoidanceVector;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	int32 MeshIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Steering Behavior Component")
	FTransform BoidTransform;
};