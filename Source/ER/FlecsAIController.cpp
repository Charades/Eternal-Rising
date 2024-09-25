// Fill out your copyright notice in the Description page of Project Settings.


#include "FlecsAIController.h"

// Sets default values
AFlecsAIController::AFlecsAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AFlecsAIController::MoveToRandomLocation()
{
	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Test"));
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		FNavLocation RandomLocation;
		const float SearchRadius = 1000.0f; // 1000 units search radius
		NavSys->GetRandomPointInNavigableRadius(ControlledPawn->GetActorLocation(), SearchRadius, RandomLocation);
		MoveToLocation(RandomLocation.Location, 100.0f);
	}
}

void AFlecsAIController::MoveToTargetLocation(FVector Loc)
{
	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Test"));
		Loc = Loc + FVector(
				   FMath::RandRange(-500.0f, 500.0f),  // Random X offset
				   FMath::RandRange(-500.0f, 500.0f),  // Random Y offset
				   0
				   );
		MoveToLocation(Loc, 100.0f);
	}
}

// Called when the game starts or when spawned
void AFlecsAIController::BeginPlay()
{
	Super::BeginPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("BeginPlay called in AI Controller"));
}

void AFlecsAIController::OnPossess(APawn *InPawn) {

	Super::OnPossess(InPawn);
	
	FVector Pos = InPawn->GetActorLocation() + FVector(100.0f, 0.0f, 0.0f);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Moving to: %s"), *Pos.ToString()));
	MoveToLocation(Pos, 10.0f);
};

// Called every frame
void AFlecsAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//MoveToRandomLocation();
}