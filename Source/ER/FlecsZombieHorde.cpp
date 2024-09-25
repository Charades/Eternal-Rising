// Fill out your copyright notice in the Description page of Project Settings.


#include "FlecsZombieHorde.h"

// Sets default values
AFlecsZombieHorde::AFlecsZombieHorde(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	AIControllerClass = AFlecsAIController::StaticClass();
	MovementComponent->MaxSpeed = 380.0f;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	SetActorEnableCollision(false);
}

// Called when the game starts or when spawned
void AFlecsZombieHorde::BeginPlay()
{
	Super::BeginPlay();
}

void AFlecsZombieHorde::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Called every frame
void AFlecsZombieHorde::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Pawn at: %s "), *GetActorLocation().ToString()));
	//
	// AFlecsAIController* AIController = Cast<AFlecsAIController>(GetController());
	// AIController->MoveToRandomLocation();
}