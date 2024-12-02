// Fill out your copyright notice in the Description page of Project Settings.


#include "FlecsZombieHorde.h"

#include "FlecsSubsystem.h"
#include "Net/UnrealNetwork.h"
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
	MovementComponent->SetIsReplicated(true);
	AIControllerClass = AFlecsAIController::StaticClass();
	MovementComponent->MaxSpeed = 380.0f;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	SetActorEnableCollision(false);
}

void AFlecsZombieHorde::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate the ISM component
	DOREPLIFETIME(AFlecsZombieHorde, InstancedMeshComponent);
	DOREPLIFETIME(AFlecsZombieHorde, InstanceTransforms);
}

// Called when the game starts or when spawned
void AFlecsZombieHorde::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicates(true);
	SetReplicateMovement(true);
	
	AAIController* CurrentController = Cast<AAIController>(GetController());
	if (CurrentController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller found: %s"), *CurrentController->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No AI Controller found. Attempting to spawn."));
	}
    
	if (HasAuthority() && GetController() == nullptr)
	{
		AAIController* NewController = GetWorld()->SpawnActor<AAIController>(AIControllerClass);
		if (NewController)
		{
			NewController->Possess(this);
			UE_LOG(LogTemp, Warning, TEXT("Spawned and possessed AI Controller"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn AI Controller"));
		}
	}
	
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
	if (HasAuthority())
	{
		// Create instance on server
		int32 IsmID = InstancedMeshComponent->AddInstance(FTransform(Rotation.Quaternion(), Location, FVector::OneVector));
        
		// Update transforms array
		UpdateInstanceTransforms();

		// Create the entity in Flecs
		auto Entity = GetEcsWorld()->entity()
			.set<FlecsHordeRef>({this})
			.set<FlecsIsmRef>({this->InstancedMeshComponent})
			.set<FlecsISMIndex>({IsmID})
			.set<FlecsZombie>({100.0f})
			.set<FlecsTargetLocation>({FVector::ZeroVector})
			.child_of<Horde>()
			.set_name(StringCast<ANSICHAR>(*FString::Printf(TEXT("Zombie%d_%d"), IsmID, this->InstancedMeshComponent->GetUniqueID())).Get());

		// Spawn on all clients
		MulticastSpawnBoid(Location, Rotation);
	}
}

void AFlecsZombieHorde::MulticastSpawnBoid_Implementation(const FVector& Location, const FRotator& Rotation)
{
	if (!HasAuthority()) // Only execute on clients
	{
		InstancedMeshComponent->AddInstance(FTransform(Rotation.Quaternion(), Location, FVector::OneVector));
	}
}

void AFlecsZombieHorde::UpdateInstanceTransforms()
{
	if (HasAuthority() && InstancedMeshComponent)
	{
		InstanceTransforms.Reset();
		const int32 InstanceCount = InstancedMeshComponent->GetInstanceCount();
		InstanceTransforms.Reserve(InstanceCount);

		for (int32 i = 0; i < InstanceCount; ++i)
		{
			FTransform Transform;
			InstancedMeshComponent->GetInstanceTransform(i, Transform, true);
			InstanceTransforms.Add(Transform);
		}
	}
}

void AFlecsZombieHorde::OnRep_InstanceTransforms()
{
	if (!HasAuthority() && InstancedMeshComponent)
	{
		// Clear existing instances
		InstancedMeshComponent->ClearInstances();

		// Add all instances from the replicated transforms
		for (const FTransform& Transform : InstanceTransforms)
		{
			InstancedMeshComponent->AddInstance(Transform);
		}
	}
}

void AFlecsZombieHorde::UpdateAndReplicateTransforms()
{
	if (!HasAuthority() || !InstancedMeshComponent) return;

	// Update the replicated transforms array
	UpdateInstanceTransforms();

	// Notify clients of the update
	if (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone)
	{
		NetMulticast_UpdateTransforms(InstanceTransforms);
	}
}

void AFlecsZombieHorde::NetMulticast_UpdateTransforms_Implementation(const TArray<FTransform>& NewTransforms)
{
	if (!HasAuthority() && InstancedMeshComponent)
	{
		// Update client-side instances
		if (NewTransforms.Num() != InstancedMeshComponent->GetInstanceCount())
		{
			InstancedMeshComponent->ClearInstances();
			for (const FTransform& Transform : NewTransforms)
			{
				InstancedMeshComponent->AddInstance(Transform);
			}
		}
		else
		{
			for (int32 i = 0; i < NewTransforms.Num(); ++i)
			{
				InstancedMeshComponent->UpdateInstanceTransform(i, NewTransforms[i], true, i == NewTransforms.Num() - 1, true);
			}
		}
	}
}

void AFlecsZombieHorde::MoveToLocation(const FVector& TargetLocation)
{
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController)
    {
        AIController->MoveToLocation(TargetLocation);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AIController is null for Horde %s"), *GetName());
    }
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
	
	MovementComponent->Velocity += FVector(0.0f, 0.0f, -1.0f) * 80.0f;
}