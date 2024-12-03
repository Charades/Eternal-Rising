// Fill out your copyright notice in the Description page of Project Settings.

#include "FlecsZombieBoid.h"

#include "AIController.h"
#include "AnimToTextureInstancePlaybackHelpers.h"
#include "SurvivorPawn.h"
#include "ZombieMeshStruct.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

AFlecsZombieBoid::AFlecsZombieBoid(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	bAutoPlay = true;
	
	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMesh"));
	RootComponent = InstancedStaticMeshComponent;
	
	InstancedStaticMeshComponent->SetRenderCustomDepth(true);

	InstancedStaticMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	
	if (FloatingPawnMovement)
	{
		FloatingPawnMovement->SetIsReplicated(true);
		FloatingPawnMovement->MaxSpeed = 245.0f;
		FloatingPawnMovement->Acceleration = 100.0f;
		FloatingPawnMovement->Deceleration = 100.0f;
		FloatingPawnMovement->TurningBoost = 2.4f;
	}
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	FlowFieldMovement = CreateDefaultSubobject<UFlowFieldMovement>(TEXT("FlowFieldMovement"));
	FlowFieldMovement->SetIsReplicated(true);

	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableObj(TEXT("/Game/Blueprints/ZombieMeshDataTable.ZombieMeshDataTable"));
	if (DataTableObj.Succeeded())
	{
		DataTable = DataTableObj.Object;
	}
}

void AFlecsZombieBoid::BeginPlay()
{
	Super::BeginPlay();

	if (!GetController())
	{
		AAIController* AIController = GetWorld()->SpawnActor<AAIController>();
		if (AIController)
		{
			AIController->Possess(this);
		}
	}

	InitializeMeshInstances();
}

void AFlecsZombieBoid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFlecsZombieBoid, FlowFieldMovement);
	DOREPLIFETIME(AFlecsZombieBoid, FloatingPawnMovement);
	DOREPLIFETIME(AFlecsZombieBoid, InstancedStaticMeshComponent);
}

void AFlecsZombieBoid::BuildAutoPlayData(UAnimToTextureDataAsset* InDA, TArray<FAnimToTextureAutoPlayData>& AutoPlayDataArray)
{
	AutoPlayDataArray.Empty();

	for (int32 i = 0; i < InstancedStaticMeshComponent->GetInstanceCount() - 1; i++)
	{
		FAnimToTextureAutoPlayData AutoPlayData;
		UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(InDA, 4, AutoPlayData, 0.0f, 1.0f);
		AutoPlayDataArray.Add(AutoPlayData);
	}
}

void AFlecsZombieBoid::BuildMatrices(TArray<FMatrix>& Matrices)
{
	Matrices.Empty();

	FTransform Transform;
	
	Matrices.Add(Transform.ToMatrixWithScale());
}

void AFlecsZombieBoid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	InitializeMeshInstances();
}

bool AFlecsZombieBoid::SetAnimation(int index)
{
	FAnimToTextureAutoPlayData AutoPlayData;
	bool bDataValid = UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(Row->DataAsset, index, AutoPlayData, 0.0f, 1.0f);
	
	if (bDataValid)
	{
		return UAnimToTextureInstancePlaybackLibrary::UpdateInstanceAutoPlayData(InstancedStaticMeshComponent, 0, AutoPlayData, true);
	}
		UE_LOG(LogTemp, Error, TEXT("Failed to get AutoPlayData from DataAsset"));
		return false;
}

void AFlecsZombieBoid::PerformAttack(AActor* Actor)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (LastAttackTime < 0 || (CurrentTime - LastAttackTime) >= AttackCooldown)
	{
		SetAnimation(4);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Attack started!"));
        
		CurrentTarget = Actor;
		LastAttackTime = CurrentTime;

		// Schedule the damage to occur after animation starts
		GetWorld()->GetTimerManager().SetTimer(
			DamageTimerHandle,
			this,
			&AFlecsZombieBoid::ApplyDelayedDamage,
			DamageDelay,
			false
		);
	}
}

void AFlecsZombieBoid::ApplyDelayedDamage()
{
	if (CurrentTarget)
	{
		if (ASurvivorPawn* SurvivorPawn = Cast<ASurvivorPawn>(CurrentTarget))
		{
			SurvivorPawn->InflictDamage(5.0f);
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Damage Applied!"));
		}
		CurrentTarget = nullptr;
	}
}

void AFlecsZombieBoid::InitializeMeshInstances()
{
	if (!DataTable) return;

	TArray<FName> RowNames = DataTable->GetRowNames();
	static const FString ContextString(TEXT("DataTableContext"));

	if (RowNames.Num() > 0)
	{
		// Randomly select a row index
		int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);

		// Get the row name at the random index
		FName RandomRowName = RowNames[RandomIndex];

		// Fetch the row using the random row name
		Row = DataTable->FindRow<FZombieMeshStruct>(RandomRowName, ContextString);
		if (Row && Row->StaticMesh)
		{
			// Set the Static Mesh for the component
			InstancedStaticMeshComponent->SetStaticMesh(Row->StaticMesh);

			// Add an instance with a fixed transform (replace with random if needed)
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(FVector(0.0f, 0.0f, 1.0f));

			BuildMatrices(_Matrices);
			UAnimToTextureInstancePlaybackLibrary::SetupInstancedMeshComponent(InstancedStaticMeshComponent, 1, bAutoPlay);
			BuildAutoPlayData(Row->DataAsset, _AutoPlayDataArray);
			UAnimToTextureInstancePlaybackLibrary::BatchUpdateInstancesAutoPlayData(InstancedStaticMeshComponent, _AutoPlayDataArray, _Matrices, true);
			SetAnimation(1);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Random Row %s has a NULL StaticMesh"), *RandomRowName.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DataTable has no rows."));
	}
}
