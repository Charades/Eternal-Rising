// Fill out your copyright notice in the Description page of Project Settings.

#include "FlecsZombieBoid.h"
#include "FlecsZombieHorde.h"
#include "FlecsZombieStimulus.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"


UFlecsZombieBoid::UFlecsZombieBoid()
	: AlignmentWeight(1.0f)
	  , CohesionWeight(1.0f)
	  , CohesionLerp(100.0f)
	  , CollisionWeight(1.0f)
	  , SeparationLerp(5.0f)
	  , SeparationForce(100.0f)
	  , SeparationWeight(0.8f)
	  , BaseMovementSpeed(150.0f)
	  , MaxMovementSpeed(250.0f)
	  , VisionRadius(400.0f)
	  , CollisionAvoidanceWeight(0),
		CollisionDistance(400.0f)
	  , MaxRotationSpeed(6.0f)
	  , MeshIndex(0)
	  , BoidTransform(FTransform::Identity)
	  , AlignmentVector(0.0)
	  , CohesionVector(0.0)
	  , SeparationVector(0.0)
	  , NegativeStimuliVector(0.0)
	  , PositiveStimuliVector(0.0)
	  , CollisionAvoidanceVector(0.0)
	  , PhysicalRadius(45.0f)
{
	PhysicalRadius2 = 2 * PhysicalRadius;
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

	if (Neighbors.Num() > 0)
	{
		ComputeCohesionVector();
		ComputeSeparationVector();
	}

	ComputeAllStimuliVectors(Horde);

	if (CollisionAvoidanceWeight != 0.0f)
	{
		ComputeCollisionAvoidanceVector(Horde);
	}

	VectorAggregation();
}

void UFlecsZombieBoid::RemoveGlobalStimulus(AFlecsZombieStimulus* Stimulus)
{
	GlobalStimulus.Remove(Stimulus);
}

void UFlecsZombieBoid::ComputeAlignmentVector()
{
	for (const UFlecsZombieBoid* Boid : Neighbors)
	{
		AlignmentVector += Boid->CurrentMoveVector.GetSafeNormal(NormalizeVectorTolerance);
	}

	AlignmentVector = (CurrentMoveVector + AlignmentVector).GetSafeNormal(NormalizeVectorTolerance) * AlignmentWeight;
}

void UFlecsZombieBoid::ComputeCohesionVector()
{
	const FVector& Location = BoidTransform.GetLocation();
	for (const UFlecsZombieBoid* Boid : Neighbors)
	{
		CohesionVector += Boid->BoidTransform.GetLocation() - Location;
	}

	CohesionVector = (CohesionVector / Neighbors.Num() / CohesionLerp) * CohesionWeight;
}

void UFlecsZombieBoid::ComputeSeparationVector()
{
	const FVector& Location = BoidTransform.GetLocation();

	for (const UFlecsZombieBoid* Boid : Neighbors)
	{
		FVector Separation = Location - Boid->BoidTransform.GetLocation();
		SeparationVector += Separation.GetSafeNormal(NormalizeVectorTolerance) / FMath::Abs(Separation.Size() - PhysicalRadius);
	}

	const FVector SeparationForceComponent = SeparationVector * SeparationForce;
	SeparationVector += (SeparationForceComponent + SeparationForceComponent * (SeparationLerp / Neighbors.Num())) * SeparationWeight;
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

bool UFlecsZombieBoid::CheckStimulusVision()
{
	static TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes{{UEngineTypes::ConvertToObjectType(ECC_Destructible)}};
	return UKismetSystemLibrary::SphereOverlapActors(
		this, BoidTransform.GetLocation(),
		VisionRadius,
		ObjectTypes,
		AFlecsZombieStimulus::StaticClass(),
		TArray<AActor*>(),
		StimulusInVision);
}

void UFlecsZombieBoid::ComputeAllStimuliVectors(AFlecsZombieHorde* Horde)
{
	CheckStimulusVision();

	const FVector& Location = BoidTransform.GetLocation();
	for (AActor* Stimulus : StimulusInVision)
	{
		ComputeStimuliComponentVector(Horde, Cast<AFlecsZombieStimulus>(Stimulus), Location);
	}
	
	for (AFlecsZombieStimulus* Stimulus : Horde->GetGlobalStimulus())
	{
		ComputeStimuliComponentVector(Horde, Stimulus, Location, true);
	}
	

	for (AFlecsZombieStimulus* Stimulus : PrivateGlobalStimulus)
	{
		ComputeStimuliComponentVector(Horde, Stimulus, Location, true);
	}
	
	NegativeStimuliVector = NegativeStimuliMaxFactor * NegativeStimuliVector.GetSafeNormal(NormalizeVectorTolerance);
}

void UFlecsZombieBoid::ComputeStimuliComponentVector(AFlecsZombieHorde* Agent, AFlecsZombieStimulus* Stimulus, const FVector& Location, bool bIsGlobal)
{
	if (!IsValid(Stimulus) || ComputedStimulus.Contains(Stimulus))
	{
		return;
	}

	ComputedStimulus.Add(Stimulus);

	if (Stimulus->Value < 0.0f)
	{
		CalculateNegativeStimuliVector(Stimulus, bIsGlobal);
	}
	else
	{
		if (FVector::Dist(Stimulus->GetActorLocation(), Location) <= (PhysicalRadius2 + Stimulus->Radius))
		{
			Stimulus->Consume(this, Agent);
		}
		else
		{
			CalculatePositiveStimuliVector(Stimulus, bIsGlobal);
		}
	}
}

void UFlecsZombieBoid::CalculateNegativeStimuliVector(const AFlecsZombieStimulus* Stimulus, bool bIsGlobal)
{
	check(Stimulus);
	const FVector Direction = Stimulus->GetActorLocation() - BoidTransform.GetLocation();
	const FVector NegativeStimuliComponentForce =
		(Direction.GetSafeNormal(NormalizeVectorTolerance)
			/ FMath::Abs(Direction.Size() - PhysicalRadius2))
		* StimuliLerp * Stimulus->Value;
	NegativeStimuliVector += NegativeStimuliComponentForce;
	NegativeStimuliMaxFactor = FMath::Max(NegativeStimuliComponentForce.Size(), NegativeStimuliMaxFactor);
}

void UFlecsZombieBoid::CalculatePositiveStimuliVector(const AFlecsZombieStimulus* Stimulus, bool bIsGlobal)
{
	check(Stimulus);
	const FVector Direction = Stimulus->GetActorLocation() - BoidTransform.GetLocation();
	const float Svalue = bIsGlobal ? Stimulus->Value : Stimulus->Value / Direction.Size();
	if (Svalue > PositiveStimuliMaxFactor)
	{
		PositiveStimuliMaxFactor = Svalue;
		PositiveStimuliVector += Stimulus->Value * Direction.GetSafeNormal(NormalizeVectorTolerance);
	}
}

void UFlecsZombieBoid::VectorAggregation()
{
	TargetMoveVector = AlignmentVector
		+ CohesionVector
		+ SeparationVector
		+ NegativeStimuliVector
		+ PositiveStimuliVector
		+ CollisionAvoidanceVector;
}

void UFlecsZombieBoid::PerformGroundTrace(AFlecsZombieHorde* Horde, float TraceDistance,
                                          ECollisionChannel CollisionChannel, float HeightOffset)
{
}
