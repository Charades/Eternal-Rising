// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlecsAIController.h"
#include "FlowFieldMovement.h"
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
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* InstancedMeshComponent;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void UpdateAndReplicateTransforms();

	UInstancedStaticMeshComponent* GetInstancedMeshComponent() { return InstancedMeshComponent; }

	UPROPERTY()
	UFloatingPawnMovement* MovementComponent;

	UPROPERTY()
	UFlowFieldMovement* FlowFieldMovement;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_InstanceTransforms)
	TArray<FTransform> InstanceTransforms;

	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// // Called to bind functionality to input
	UFUNCTION()
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void SpawnBoid(const FVector& Location, const FRotator& Rotation);

	flecs::world* GetEcsWorld() const;

	UFUNCTION()
	void MoveToLocation(const FVector& TargetLocation);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnBoid(const FVector& Location, const FRotator& Rotation);
	
	UFUNCTION()
	void OnRep_InstanceTransforms();

	UFUNCTION()
	void UpdateInstanceTransforms();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UpdateTransforms(const TArray<FTransform>& NewTransforms);
	
private:
	struct FTransformUpdate
	{
		int32 Index;
		FTransform Transform;
	};
};