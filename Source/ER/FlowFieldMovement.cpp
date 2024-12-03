#include "FlowFieldMovement.h"

#include "FlecsZombieBoid.h"
#include "SurvivorPawn.h"
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UFlowFieldMovement::UFlowFieldMovement(): Move(false), WanderRadius(250.0f), bDestinationReached(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
	// ...
}


// Called when the game starts
void UFlowFieldMovement::BeginPlay()
{
	Super::BeginPlay();

	FlowFieldActor = Cast<AFlowFieldWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AFlowFieldWorld::StaticClass()));
	OwnerPawn = Cast<AFlecsZombieBoid>(GetOwner());
	FloatingPawnMovement = OwnerPawn ? OwnerPawn->FindComponentByClass<UFloatingPawnMovement>() : nullptr;

	// Setup initial wandering parameters
	WanderCenterPoint = FlowFieldActor ? FlowFieldActor->GetActorLocation() : FVector::ZeroVector;
	WanderRadius = 900.0f;
}

void UFlowFieldMovement::BeginMovement(TMap<FVector2D, FVector>& InDirectionMap, FVector& InGoalPosition)
{
	ClearTargetEnemy();
	DirectionMap = InDirectionMap;
	GoalPosition = InGoalPosition;
	Move = true;
	bDestinationReached = false;
}

void UFlowFieldMovement::CheckNeighborsDestinationStatus()
{
	TArray<APawn*> Neighbors = GetNeighbors();
	bool bAllNeighborsReachedDestination = true;

	for (APawn* Neighbor : Neighbors)
	{
		UFlowFieldMovement* NeighborMovement = Neighbor ? Neighbor->FindComponentByClass<UFlowFieldMovement>() : nullptr;
		if (NeighborMovement && !NeighborMovement->bDestinationReached)
		{
			bAllNeighborsReachedDestination = false;
			break;
		}
	}

	// If all neighbors have reached destination, switch to wandering
	if (bAllNeighborsReachedDestination)
	{
		Move = false;
		bDestinationReached = true;
	}
}

FVector UFlowFieldMovement::GetGoalSeekingDirection()
{
	// Original goal-seeking direction logic
	FVector2D CurrentCell = FindCurrentCell(OwnerPawn->GetActorLocation());
	FVector FlowFieldDirection;

	if (FVector* FoundDirection = DirectionMap.Find(CurrentCell))
	{
		FlowFieldDirection = *FoundDirection;
		if (FlowFieldDirection.IsNearlyZero())
		{
			FlowFieldDirection = (GoalPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
		}
	}
	else
	{
		FlowFieldDirection = (GoalPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
	}

	// Calculate boid forces
	TArray<APawn*> Neighbors = GetNeighbors();
	FVector AlignmentForce = CalculateAlignment(Neighbors);
	FVector CohesionForce = CalculateCohesion(Neighbors);
	FVector SeparationForce = CalculateSeparation(Neighbors);

	return (
		FlowFieldDirection * FlowFieldWeight +
		AlignmentForce * AlignmentWeight +
		CohesionForce * CohesionWeight +
		SeparationForce * SeparationWeight
	).GetSafeNormal();
}

FVector UFlowFieldMovement::GetWanderingDirection()
{
    if (!FlowFieldActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("FlowFieldActor is null in GetWanderingDirection"));
        return FVector::ZeroVector;
    }

	FindAndTargetNearbyEnemy();
	
    // Calculate grid boundaries
    float MinX = GoalPosition.X - WanderRadius;
    float MaxX = GoalPosition.X + WanderRadius;
    float MinY = GoalPosition.Y - WanderRadius;
    float MaxY = GoalPosition.Y + WanderRadius;

	FVector BoxExtent = FVector(
	FMath::Abs(MaxX - MinX) / 2.0f, 
	FMath::Abs(MaxY - MinY) / 2.0f, 
	200.0f 
	);

	// Draw debug box for wander area
	// DrawDebugBox(
	// 	GetWorld(),
	// 	GoalPosition,
	// 	BoxExtent,
	// 	FQuat::Identity,
	// 	FColor::Green,
	// 	false,  // Persistent
	// 	0.1f,   // Lifetime
	// 	0,      // Depth priority
	// 	2.0f    // Thickness
	// );
	
    // Get current pawn location
    FVector CurrentLocation = OwnerPawn->GetActorLocation();

    // Improved Boundary Handling
    bool bAtXBoundary = CurrentLocation.X <= MinX || CurrentLocation.X >= MaxX;
    bool bAtYBoundary = CurrentLocation.Y <= MinY || CurrentLocation.Y >= MaxY;

    // If stuck at a boundary, force a new wander target away from the boundary
    if (bAtXBoundary || bAtYBoundary)
    {
        float RandomX, RandomY;
        
        // If at X boundary, generate X in the opposite direction
        if (CurrentLocation.X <= MinX)
        {
            RandomX = FMath::RandRange(MinX + WanderRadius/2, MaxX);
        }
        else if (CurrentLocation.X >= MaxX)
        {
            RandomX = FMath::RandRange(MinX, MaxX - WanderRadius/2);
        }
        else
        {
            RandomX = FMath::RandRange(MinX, MaxX);
        }

        // If at Y boundary, generate Y in the opposite direction
        if (CurrentLocation.Y <= MinY)
        {
            RandomY = FMath::RandRange(MinY + WanderRadius/2, MaxY);
        }
        else if (CurrentLocation.Y >= MaxY)
        {
            RandomY = FMath::RandRange(MinY, MaxY - WanderRadius/2);
        }
        else
        {
            RandomY = FMath::RandRange(MinY, MaxY);
        }

        // Set new wander target
        WanderTarget = FVector(RandomX, RandomY, GoalPosition.Z);
    }
    
    // Original logic for generating wander target if not set
    if (WanderTarget == FVector::ZeroVector || 
        FVector::Distance(OwnerPawn->GetActorLocation(), WanderTarget) < GoalAcceptanceDist)
    {
        // Generate a random point within the grid boundaries
        float RandomX = FMath::RandRange(MinX, MaxX);
        float RandomY = FMath::RandRange(MinY, MaxY);
        WanderTarget = FVector(RandomX, RandomY, GoalPosition.Z);
    }

    // Calculate wander direction
    FVector WanderDirection = (WanderTarget - CurrentLocation).GetSafeNormal();

    // Calculate boid forces
    TArray<APawn*> Neighbors = GetNeighbors();
    FVector AlignmentForce = CalculateAlignment(Neighbors);
    FVector CohesionForce = CalculateCohesion(Neighbors);
    FVector SeparationForce = CalculateSeparation(Neighbors);

    // Combine forces with boundary correction
    FVector CombinedDirection = (
        WanderDirection * FlowFieldWeight +
        AlignmentForce * AlignmentWeight +
        CohesionForce * CohesionWeight +
        SeparationForce * SeparationWeight
    ).GetSafeNormal();

    // Boundary correction force
    FVector BoundaryCorrection = FVector::ZeroVector;
    
    // Check X boundaries
    if (CurrentLocation.X < MinX)
    {
        BoundaryCorrection.X = FMath::Abs(MinX - CurrentLocation.X);
    }
    else if (CurrentLocation.X > MaxX)
    {
        BoundaryCorrection.X = -FMath::Abs(CurrentLocation.X - MaxX);
    }

    // Check Y boundaries
    if (CurrentLocation.Y < MinY)
    {
        BoundaryCorrection.Y = FMath::Abs(MinY - CurrentLocation.Y);
    }
    else if (CurrentLocation.Y > MaxY)
    {
        BoundaryCorrection.Y = -FMath::Abs(CurrentLocation.Y - MaxY);
    }

    // Scale boundary correction
    BoundaryCorrection = BoundaryCorrection.GetSafeNormal(); // Adjust multiplier as needed

    // Combine direction with boundary correction
    FVector FinalDirection = (CombinedDirection + BoundaryCorrection).GetSafeNormal();

    // Optional: Debug visualization
    // DrawDebugDirectionalArrow(
    //     GetWorld(),
    //     CurrentLocation,
    //     CurrentLocation + FinalDirection * 200.0f,
    //     50.0f,  // Arrow size
    //     FColor::Blue,
    //     false,  // Persistent
    //     0.1f,   // Lifetime
    //     0,      // Depth priority
    //     5.0f    // Thickness
    // );

    return FinalDirection;
}

void UFlowFieldMovement::ApplyMovementAndRotation(FVector DesiredDirection, float DeltaTime)
{
    FRotator CurrentRotation = OwnerPawn->GetActorRotation();
    FRotator TargetRotation = DesiredDirection.Rotation();
    FVector ForwardVector = TargetRotation.Vector();
    TargetRotation.Yaw -= 90.0f;
    TargetRotation.Normalize();

    // Obstacle Detection with Multiple Directions
    FVector Start = OwnerPawn->GetActorLocation();
    Start.Z = 50.0f;
    
    float LineTraceDistance = 200.0f;
    TArray<FVector> CheckDirections = {
        ForwardVector,
        FRotationMatrix(FRotator(0, 45, 0)).TransformVector(ForwardVector),
        FRotationMatrix(FRotator(0, -45, 0)).TransformVector(ForwardVector)
    };

    FVector ObstacleAvoidanceForce = FVector::ZeroVector;
    bool bHitObstacle = false;

    for (const FVector& CheckDirection : CheckDirections)
    {
        FVector End = Start + CheckDirection * LineTraceDistance;

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwnerPawn);

        if (GetWorld()->LineTraceSingleByChannel(
            HitResult, 
            Start, 
            End, 
            ECC_Visibility, 
            QueryParams
        ))
        {
            bHitObstacle = true;

            // Determine alternative navigation direction
            FVector AvoidanceDirection = FVector::CrossProduct(HitResult.ImpactNormal, FVector::UpVector).GetSafeNormal();
            
            // Ensure avoidance is away from the obstacle
            float DotProduct = FVector::DotProduct(AvoidanceDirection, ForwardVector);
            if (DotProduct < 0)
            {
                AvoidanceDirection *= -1;
            }

            // Calculate avoidance intensity based on proximity
            float DistanceToObstacle = HitResult.Distance;
            float AvoidanceIntensity = FMath::Clamp(1.0f - (DistanceToObstacle / LineTraceDistance), 0.0f, 1.0f);
            
            // Strong avoidance force when close to obstacle
            ObstacleAvoidanceForce = AvoidanceDirection * AvoidanceIntensity * 75.0f;
            DesiredDirection = (DesiredDirection + ObstacleAvoidanceForce).GetSafeNormal();
        }

        // Debug Line Trace Visualization
        FColor LineColor = bHitObstacle ? FColor::Red : FColor::Green;
         DrawDebugLine(
             GetWorld(), 
             Start, 
             End, 
             LineColor, 
             false,  
             -1.0f,  
             0,      
             2.0f    
         );
    }

    // Calculate dynamic interpolation speed
    TargetRotation = DesiredDirection.Rotation();
    TargetRotation.Yaw -= 90.0f;
    TargetRotation.Normalize();

    float AngleDifference = FMath::Abs(CurrentRotation.Yaw - TargetRotation.Yaw);
    float InterpSpeed = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 180.0f), FVector2D(1.0f, 2.0f), AngleDifference);

    // Add slight randomness
    float RandomSway = FMath::FRandRange(-0.5f, 0.5f);
    TargetRotation.Yaw += RandomSway;
    TargetRotation.Normalize();

    // Smooth interpolation
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
    NewRotation.Pitch = CurrentRotation.Pitch;
    NewRotation.Roll = CurrentRotation.Roll;

    OwnerPawn->SetActorRotation(NewRotation);

    // Apply movement
    OwnerPawn->AddMovementInput(DesiredDirection, 1.0f, false);

    // Optional: Gravity application (if needed)
    FloatingPawnMovement->Velocity += FVector(0.0f, 0.0f, -1.0f) * 80.0f;
}

FVector2D UFlowFieldMovement::FindCurrentCell(FVector InPawnWorldPos)
{
	if (!FlowFieldActor)
	{
		return FVector2D::ZeroVector;
	}

	// Get relative position to flow field grid
	FVector RelativePos = InPawnWorldPos - FlowFieldActor->GetActorLocation();
	float CellSize = FlowFieldActor->cellSize;

	// Calculate grid coordinates
	int32 GridX = FMath::FloorToInt(RelativePos.X / CellSize);
	int32 GridY = FMath::FloorToInt(RelativePos.Y / CellSize);

	// Ensure we stay within grid bounds
	GridX = FMath::Clamp(GridX, 0, FlowFieldActor->xAmount - 1);
	GridY = FMath::Clamp(GridY, 0, FlowFieldActor->yAmount - 1);
	
	return FVector2D(GridX, GridY);
}

// TArray<APawn*> UFlowFieldMovement::GetNeighbors()
// {
// 	TArray<APawn*> Neighbors;
// 	if (!OwnerPawn)
// 		return Neighbors;
//
// 	TArray<AActor*> AllPawns;
// 	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);
//
// 	for (AActor* Actor : AllPawns)
// 	{
// 		APawn* OtherPawn = Cast<APawn>(Actor);
// 		if (OtherPawn && OtherPawn != OwnerPawn)
// 		{
// 			float Distance = FVector::Distance(OwnerPawn->GetActorLocation(), OtherPawn->GetActorLocation());
// 			if (Distance <= PerceptionRadius)
// 			{
// 				Neighbors.Add(OtherPawn);
// 			}
// 		}
// 	}
//
// 	return Neighbors;
// }

void UFlowFieldMovement::SetExternalNeighbors(const TArray<APawn*>& InNeighbors)
{
	ExternalNeighbors = InNeighbors;
	bUseExternalNeighbors = true;
}

TArray<APawn*> UFlowFieldMovement::GetNeighbors()
{
	// If external neighbors are set, use them
	if (bUseExternalNeighbors && !ExternalNeighbors.IsEmpty())
	{
		return ExternalNeighbors;
	}

	// Fallback to original neighbor detection method
	TArray<APawn*> Neighbors;
	if (!OwnerPawn)
		return Neighbors;

	// Use sphere overlap method as you already have
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerPawn);

	TArray<FOverlapResult> OverlapResults;
	bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		OwnerPawn->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(PerceptionRadius),
		QueryParams
	);

	if (bHasOverlaps)
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (APawn* OtherPawn = Cast<APawn>(Overlap.GetActor()))
			{
				if (OtherPawn != OwnerPawn)
				{
					Neighbors.Add(OtherPawn);
				}
			}
		}
	}

	return Neighbors;
}

FVector UFlowFieldMovement::CalculateAlignment(const TArray<APawn*>& Neighbors)
{
	if (Neighbors.Num() == 0)
		return FVector::ZeroVector;

	FVector AverageVelocity = FVector::ZeroVector;
	for (APawn* Neighbor : Neighbors)
	{
		if (UFloatingPawnMovement* Movement = Neighbor->FindComponentByClass<UFloatingPawnMovement>())
		{
			AverageVelocity += Movement->Velocity;
		}
	}

	return (AverageVelocity / Neighbors.Num()).GetSafeNormal();
}

FVector UFlowFieldMovement::CalculateCohesion(const TArray<APawn*>& Neighbors)
{
	if (Neighbors.Num() == 0)
		return FVector::ZeroVector;

	FVector CenterOfMass = FVector::ZeroVector;
	for (APawn* Neighbor : Neighbors)
	{
		CenterOfMass += Neighbor->GetActorLocation();
	}

	CenterOfMass /= Neighbors.Num();
	return (CenterOfMass - OwnerPawn->GetActorLocation()).GetSafeNormal();
}

FVector UFlowFieldMovement::CalculateSeparation(const TArray<APawn*>& Neighbors)
{
	if (Neighbors.Num() == 0)
		return FVector::ZeroVector;

	FVector SeparationForce = FVector::ZeroVector;
	for (APawn* Neighbor : Neighbors)
	{
		FVector AwayFromNeighbor = OwnerPawn->GetActorLocation() - Neighbor->GetActorLocation();
		float Distance = AwayFromNeighbor.Size();
		if (Distance > 0)
		{
			SeparationForce += AwayFromNeighbor.GetSafeNormal() / Distance;
		}
	}

	return SeparationForce.GetSafeNormal();
}

void UFlowFieldMovement::CheckAndPerformAttack()
{
    if (!CurrentTargetEnemy || !OwnerPawn)
        return;

    float DistanceToTarget = FVector::Distance(OwnerPawn->GetActorLocation(), CurrentTargetEnemy->GetActorLocation());
    
    // Attack range (adjust as needed)
    const float AttackRange = 120.0f;

    if (DistanceToTarget <= AttackRange)
    {
        FVector Start = OwnerPawn->GetActorLocation();
        Start.Z = 50.0f;
        
        float LineTraceDistance = 200.0f;
        FVector ForwardVector = (CurrentTargetEnemy->GetActorLocation() - Start).GetSafeNormal();
        
        TArray<FVector> CheckDirections = {
            ForwardVector,
            FRotationMatrix(FRotator(0, 45, 0)).TransformVector(ForwardVector),
            FRotationMatrix(FRotator(0, -45, 0)).TransformVector(ForwardVector)
        };

        for (const FVector& CheckDirection : CheckDirections)
        {
            FVector End = Start + CheckDirection * LineTraceDistance;

            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(OwnerPawn);

            if (GetWorld()->LineTraceSingleByChannel(
                HitResult, 
                Start, 
                End, 
                ECC_Visibility, 
                QueryParams
            ))
            {
                // If the hit actor is the target enemy, perform attack
                if (HitResult.GetActor() == CurrentTargetEnemy)
                {
                	OwnerPawn->PerformAttack(CurrentTargetEnemy);
                }
            }
        }
    }
}

void UFlowFieldMovement::SetTargetEnemy(AActor* Enemy)
{
    CurrentTargetEnemy = Enemy;
}

void UFlowFieldMovement::ClearTargetEnemy()
{
	CurrentTargetEnemy = nullptr;
}

void UFlowFieldMovement::FindAndTargetNearbyEnemy()
{
	FVector Start = OwnerPawn->GetActorLocation();
	Start.Z = 50.0f;
    
	float LineTraceDistance = 500.0f; // Adjust detection range as needed
	FVector ForwardVector = OwnerPawn->GetActorRotation().Vector();
    
	TArray<FVector> CheckDirections = {
		ForwardVector,
		FRotationMatrix(FRotator(0, 45, 0)).TransformVector(ForwardVector),
		FRotationMatrix(FRotator(0, -45, 0)).TransformVector(ForwardVector)
	};

	for (const FVector& CheckDirection : CheckDirections)
	{
		FVector End = Start + CheckDirection * LineTraceDistance;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwnerPawn);

		if (GetWorld()->LineTraceSingleByChannel(
			HitResult, 
			Start, 
			End, 
			ECC_Pawn, 
			QueryParams
		))
		{
			// If the hit actor is a survivor, set as target
			if (ASurvivorPawn* SurvivorPawn = Cast<ASurvivorPawn>(HitResult.GetActor()))
			{
				SetTargetEnemy(SurvivorPawn);
				OwnerPawn->PerformAttack(SurvivorPawn);
			}
		}
	}
}


void UFlowFieldMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerPawn || !FloatingPawnMovement)
        return;
    
    // Prioritize enemy targeting if an enemy is set
    if (CurrentTargetEnemy)
    {
        float DistanceToEnemy = FVector::Distance(OwnerPawn->GetActorLocation(), CurrentTargetEnemy->GetActorLocation());
        
        // If enemy is close enough, perform attack
        if (DistanceToEnemy <= GoalAcceptanceDist)
        {
            Move = false;
            bDestinationReached = true;
        	OwnerPawn->PerformAttack(CurrentTargetEnemy);
            return;
        }

        // Move towards the enemy
        GoalPosition = CurrentTargetEnemy->GetActorLocation();
        Move = true;
        bDestinationReached = false;
    	OwnerPawn->SetAnimation(0);
        
        // Get direction to enemy
        FVector DesiredDirection = (GoalPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
        
        // Apply movement towards enemy
        ApplyMovementAndRotation(DesiredDirection, DeltaTime);
        return;
    }

    // Check if we've reached the goal
    float DistanceToGoal = FVector::Distance(OwnerPawn->GetActorLocation(), GoalPosition);
    if (DistanceToGoal <= GoalAcceptanceDist)
    {
        Move = false;
        bDestinationReached = true;
    }

    // Check neighbors' destination status when destination is reached
    if (bDestinationReached)
    {
        CheckNeighborsDestinationStatus();
    }

    // Determine movement strategy
    FVector DesiredDirection;
    if (Move)
    {
        // Original goal-seeking behavior
        DesiredDirection = GetGoalSeekingDirection();
        OwnerPawn->SetAnimation(0);
    }
    else if (bDestinationReached)
    {
        // Always allow wandering once destination is reached
        DesiredDirection = GetWanderingDirection();
        OwnerPawn->SetAnimation(0);
    }
    else
    {
        return; // No movement
    }

    // Apply movement and rotation
    ApplyMovementAndRotation(DesiredDirection, DeltaTime);
}