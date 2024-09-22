// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
//#include "FlecsSubsystem.h"
#include "flecs.h"
#include "FlecsZombieHorde.generated.h"

UCLASS()
class ER_API AFlecsZombieHorde : public APawn
{
	GENERATED_BODY()

public:
	AFlecsZombieHorde();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInstancedStaticMeshComponent* InstancedMeshComponent;

private:
	TArray<int32> InstanceIndices;
	TArray<flecs::entity> SquadEntities;
	
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
