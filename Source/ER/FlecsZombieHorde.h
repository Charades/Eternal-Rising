// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlecsAIController.h"
#include "flecs.h"
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
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};