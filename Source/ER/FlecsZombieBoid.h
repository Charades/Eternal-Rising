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

	void RemoveGlobalStimulus(AFlecsZombieStimulus* Stimulus);
	
protected:
	void ComputeMovementVector(AFlecsZombieHorde* HordeAgent);
	void ComputeAlignmentVector();
	bool CheckStimulusVision();
	void ComputeCohesionVector();
	void ComputeSeparationVector();
	void ComputeCollisionAvoidanceVector(AFlecsZombieHorde* HordeAgent);
	void ComputeAllStimuliVectors(AFlecsZombieHorde* HordeAgent);
	void ComputeStimuliComponentVector(AFlecsZombieHorde* Agent, AFlecsZombieStimulus* Stimulus, const FVector& Location, bool bIsGlobal = false);
	void CalculateNegativeStimuliVector(const AFlecsZombieStimulus* Stimulus, bool bIsGlobal = false);
	void CalculatePositiveStimuliVector(const AFlecsZombieStimulus* Stimulus, bool bIsGlobal = false);
	void VectorAggregation();
	void PerformGroundTrace(AFlecsZombieHorde* HordeAgent, float TraceDistance, ECollisionChannel CollisionChannel = ECC_WorldStatic, float HeightOffset = 35.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	FVector TargetMoveVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking Component")
	FVector CurrentMoveVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	FVector AlignmentVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	FVector CohesionVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	FVector SeparationVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	FVector NegativeStimuliVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	FVector PositiveStimuliVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	FVector CollisionAvoidanceVector;
	
	TSet<AFlecsZombieStimulus*> StimulusSet;

	TArray<AFlecsZombieStimulus*> GlobalStimulus;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	TArray<UFlecsZombieBoid*> Neighbors;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	int32 MeshIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flocking Component")
	FTransform BoidTransform;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float BaseMovementSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float MaxMovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float MaxRotationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float VisionRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	bool bAlignWithFloor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float FloorHeightOffset = 24.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component", meta = (Tooltip = "Max distance to the floor at ground level", EditCondition = "bAlignWithFloor"))
	float MaxFloorDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float CollisionAvoidanceWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float AlignmentWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float CohesionWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float CohesionLerp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float SeparationWeight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float SeparationForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float SeparationLerp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float CollisionWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float CollisionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	double CollisionDeviationAngle = PI * 10.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float PhysicalRadius;

	UPROPERTY(VisibleAnywhere , BlueprintReadOnly, Category = "Flocking Component")
	TArray<AActor*> StimulusInVision;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float NegativeStimuliMaxFactor;

	UPROPERTY(EditAnywhere , BlueprintReadWrite, Category = "AI|Steering Behavior Component")
	float PositiveStimuliMaxFactor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking Component")
	float StimuliLerp;



protected:
	float PhysicalRadius2;
	
	TArray<AFlecsZombieStimulus*> PrivateGlobalStimulus;
	
	TSet<AFlecsZombieStimulus*> ComputedStimulus;
	
	const float NormalizeVectorTolerance = 0.0001f;
};