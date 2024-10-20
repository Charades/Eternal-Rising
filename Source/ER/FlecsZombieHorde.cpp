﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "FlecsZombieHorde.h"

#include "FlecsSubsystem.h"
#include "FlecsZombieBoid.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AFlecsZombieHorde::AFlecsZombieHorde(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InstancedMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMeshComponent"));
	RootComponent = InstancedMeshComponent;
	InstancedMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
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

	UInstancedStaticMeshComponent* ZombieRenderer = FindComponentByClass<UInstancedStaticMeshComponent>();
	if (ZombieRenderer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Instanced Mesh Component found."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Instanced Mesh Component not found."));
	}
}

void AFlecsZombieHorde::SpawnBoid(const FVector& Location, const FRotator& Rotation)
{
	//Create new instanced mesh in location and rotation
	int32 IsmID = InstancedMeshComponent->AddInstance(FTransform(Rotation.Quaternion(), Location, FVector::OneVector));
	UFlecsZombieBoid* Boid = NewObject<UFlecsZombieBoid>(this);
	Boid->Init(Location, Rotation, IsmID);
	{
		FScopeLock ScopeLock(&MutexBoid);
		Boids.Add(IsmID, Boid);
	}
	
	// Create the entity in Flecs
	auto Entity = GetEcsWorld()->entity()
		.set<FlecsHordeRef>({this})
		.set<FlecsIsmRef>({this->InstancedMeshComponent})
		.set<FlecsISMIndex>({IsmID})
		.set<FlecsZombie>({100.0f})
		.set<FlecsTargetLocation>({FVector::ZeroVector})
		.child_of<Horde>()
		.set_name(StringCast<ANSICHAR>(*FString::Printf(TEXT("Zombie%d_%d"), IsmID, this->InstancedMeshComponent->GetUniqueID())).Get());
}

flecs::world* AFlecsZombieHorde::GetEcsWorld() const
{
	// Get the game instance and then the FlecsSubsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance)
	{
		UFlecsSubsystem* FlecsSubsystem = GameInstance->GetSubsystem<UFlecsSubsystem>();
		if (FlecsSubsystem)
		{
			// Call the function to get the ECS world
			return FlecsSubsystem->GetEcsWorld();
		}
	}
    
	// Return nullptr if anything goes wrong
	return nullptr;
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