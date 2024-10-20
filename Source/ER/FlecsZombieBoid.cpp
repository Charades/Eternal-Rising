// Fill out your copyright notice in the Description page of Project Settings.

#include "FlecsZombieBoid.h"
#include "FlecsZombieHorde.h"
#include "FlecsZombieStimulus.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"


UFlecsZombieBoid::UFlecsZombieBoid()
{
}

void UFlecsZombieBoid::ResetComponents()
{
	AlignmentVector = FVector::ZeroVector;
	CohesionVector = FVector::ZeroVector;
	SeparationVector = FVector::ZeroVector;
	NegativeStimuliVector = FVector::ZeroVector;
	PositiveStimuliVector = FVector::ZeroVector;
	CollisionAvoidanceVector = FVector::ZeroVector;
	StimulusSet.Empty(StimulusSet.Num());
}

void UFlecsZombieBoid::Init(const FVector& Location, const FRotator& Rotation, int32 MeshInstanceIndex)
{
	BoidTransform.SetRotation(Rotation.Quaternion());
	BoidTransform.SetLocation(Location);
	MeshIndex = MeshInstanceIndex;
	TargetMoveVector = Rotation.Vector().GetSafeNormal();
}

void UFlecsZombieBoid::Update(float DeltaSeconds, AFlecsZombieHorde* Horde)
{
	CurrentMoveVector = TargetMoveVector;
	ComputeMovementVector(Horde);
	
	const FVector NewDirection = (TargetMoveVector * BaseMovementSpeed * DeltaSeconds).GetClampedToMaxSize(MaxMovementSpeed * DeltaSeconds);
	BoidTransform.SetLocation(BoidTransform.GetLocation() + NewDirection);
	BoidTransform.SetRotation(
		UKismetMathLibrary::RLerp(
			BoidTransform.Rotator(),
			UKismetMathLibrary::MakeRotFromXZ(NewDirection, FVector::UpVector),
			DeltaSeconds * MaxRotationSpeed, false).Quaternion());
	if (bAlignWithFloor)
	{
		PerformGroundTrace(Horde, MaxFloorDistance, ECC_WorldStatic, FloorHeightOffset);
	}
}

void UFlecsZombieBoid::ComputeMovementVector(AFlecsZombieHorde* Horde)
{
	ResetComponents();
	ComputeAlignmentVector();

	if (NeighboringBoids.Num() > 0)
	{
		ComputeCohesionVector();
		ComputeSeparationVector();
	}

	ComputeAllStimuliVectors(Horde);

	if (CollisionAvoidanceWeight != 0.0f)
	{
		ComputeCollisionAvoidanceVector(Horde);
	}
}

void UFlecsZombieBoid::ComputeAlignmentVector()
{
	for (const UFlecsZombieBoid* Boid : NeighboringBoids)
	{
		AlignmentVector += Boid->CurrentMoveVector.GetSafeNormal(NormalizeVectorTolerance);
	}

	AlignmentVector = (CurrentMoveVector + AlignmentVector).GetSafeNormal(NormalizeVectorTolerance) * AlignmentWeight;
}

void UFlecsZombieBoid::ComputeCohesionVector()
{
	const FVector& Location = BoidTransform.GetLocation();
	for (const UFlecsZombieBoid* Boid : NeighboringBoids)
	{
		CohesionVector += Boid->BoidTransform.GetLocation() - Location;
	}

	CohesionVector = (CohesionVector / NeighboringBoids.Num() / CohesionLerp) * CohesionWeight;
}

void UFlecsZombieBoid::ComputeSeparationVector()
{
	const FVector& Location = BoidTransform.GetLocation();

	for (const UFlecsZombieBoid* Boid : NeighboringBoids)
	{
		FVector Separation = Location - Boid->BoidTransform.GetLocation();
		SeparationVector += Separation.GetSafeNormal(NormalizeVectorTolerance) / FMath::Abs(Separation.Size() - PhysicalRadius);
	}

	const FVector SeparationForceComponent = SeparationVector * SeparationForce;
	SeparationVector += (SeparationForceComponent + SeparationForceComponent * (SeparationLerp / NeighboringBoids.Num())) * SeparationWeight;
}

void UFlecsZombieBoid::ComputeCollisionAvoidanceVector(AFlecsZombieHorde* Horde)
{
	FHitResult OutHit;
	const FVector& Location = BoidTransform.GetLocation();
	static const FName LineTrace(TEXT("LineTrace"));
	const FVector End = Location + BoidTransform.GetRotation().GetForwardVector() * CollisionDistance;
	FCollisionQueryParams Params(LineTrace, false);

	const static FCollisionShape SphereShape = FCollisionShape::MakeSphere(PhysicalRadius);
	
	if (GetWorld()->SweepSingleByChannel(OutHit, Location, End, FQuat::Identity, ECC_WorldStatic, SphereShape, Params))
	{
		const FVector Direction = OutHit.ImpactPoint - BoidTransform.GetLocation();
		CollisionAvoidanceVector -= (Direction.GetSafeNormal(NormalizeVectorTolerance) / FMath::Abs(Direction.Size() - PhysicalRadius)).RotateAngleAxis(CollisionDeviationAngle, FVector::UpVector) * CollisionWeight;
	}
}

void UFlecsZombieBoid::ComputeAllStimuliVectors(AFlecsZombieHorde* Horde)
{
}

void UFlecsZombieBoid::PerformGroundTrace(AFlecsZombieHorde* Horde, float TraceDistance,
                                          ECollisionChannel CollisionChannel, float HeightOffset)
{
}
