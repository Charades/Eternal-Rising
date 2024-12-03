// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnActor.h"

// Sets default values
ASurvivorPawn::ASurvivorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

// Called when the game starts or when spawned
void ASurvivorPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASurvivorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASurvivorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASurvivorPawn::InflictDamage(float DamageAmount)
{
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
		FString::Printf(TEXT("Health: %.1f"), CurrentHealth));
}