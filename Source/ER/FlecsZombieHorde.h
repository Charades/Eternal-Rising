// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlecsAIController.h"
#include "flecs.h"
#include "FlecsZombieBoid.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "FlecsZombieHorde.generated.h"

UCLASS()
class ER_API AFlecsZombieHorde : public APawn
{
	GENERATED_BODY()

public:
	AFlecsZombieHorde(const class FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* InstancedMeshComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFloatingPawnMovement* MovementComponent;

	// All the agents are now boids inside this Agents Manager
	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TMap<int32, UFlecsZombieBoid*> Boids;

	//protect the use of the boids
	FCriticalSection MutexBoid;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void SpawnBoid(const FVector& Location, const FRotator& Rotation);
	flecs::world* GetEcsWorld() const;
};