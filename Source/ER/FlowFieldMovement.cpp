// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldMovement.h"

// Sets default values for this component's properties
UFlowFieldMovement::UFlowFieldMovement(): Move(false)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFlowFieldMovement::BeginPlay()
{
	Super::BeginPlay();

	FlowFieldActor = Cast<AFlowFieldWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), AFlowFieldWorld::StaticClass()));

	if (!FlowFieldActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("FlowFieldCCP actor not found."));
	}
	
	OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner is not a Pawn."));
	}

	FloatingPawnMovement = OwnerPawn->FindComponentByClass<UFloatingPawnMovement>();

	if (FloatingPawnMovement)
	{
		UE_LOG(LogTemp, Log, TEXT("FloatingPawnMovement successfully updated."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FloatingPawnMovement component not found on the Pawn."));
	}
}

void UFlowFieldMovement::BeginMovement(TMap<FVector2D, FVector>& InDirectionMap, FVector& InGoalPosition)
{
	DirectionMap = InDirectionMap;
	GoalPosition = InGoalPosition;
	Move = true;
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

// Called every frame
void UFlowFieldMovement::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Move || !OwnerPawn || !FloatingPawnMovement)
	{
		return;
	}

	// Apply gravity
	FloatingPawnMovement->Velocity += FVector(0.0f, 0.0f, -1.0f) * 80.0f;

	// Check if we've reached the goal
	float DistanceToGoal = FVector::Distance(OwnerPawn->GetActorLocation(), GoalPosition);
	if (DistanceToGoal <= GoalAcceptanceDist)
	{
		Move = false;
		return;
	}

	// Get current grid cell and look up movement direction
	FVector2D CurrentCell = FindCurrentCell(OwnerPawn->GetActorLocation());
	FVector DesiredDirection;

	if (FVector* FoundDirection = DirectionMap.Find(CurrentCell))
	{
		DesiredDirection = *FoundDirection;
        
		// If the direction is zero or very small, fall back to direct path
		if (DesiredDirection.IsNearlyZero())
		{
			DesiredDirection = (GoalPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
		}
	}
	else
	{
		// Fallback to direct path if no flow field direction found
		DesiredDirection = (GoalPosition - OwnerPawn->GetActorLocation()).GetSafeNormal();
	}

	// Smooth rotation
	FRotator CurrentRotation = OwnerPawn->GetActorRotation();
	FRotator TargetRotation = DesiredDirection.Rotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
    
	// Only rotate on Z axis
	NewRotation.Pitch = CurrentRotation.Pitch;
	NewRotation.Roll = CurrentRotation.Roll;
    
	OwnerPawn->SetActorRotation(NewRotation);

	// Apply movement
	OwnerPawn->AddMovementInput(DesiredDirection, 1.0f, false);
}

