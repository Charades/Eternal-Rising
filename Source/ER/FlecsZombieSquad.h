// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "flecs.h"
#include "FlecsZombieSquad.generated.h"

UCLASS()
class ER_API AFlecsZombiePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AFlecsZombiePawn();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* InstancedMeshComponent;

private:
	TArray<int32> InstanceIndices; // Store indices of instanced static meshes for this squad
	TArray<flecs::entity> SquadEntities; // Flecs entities for this squad
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void AddEntityToSquad(flecs::entity Entity, int32 InstanceIndex);
};
