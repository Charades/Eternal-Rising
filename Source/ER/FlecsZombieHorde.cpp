// Fill out your copyright notice in the Description page of Project Settings.


#include "FlecsZombieHorde.h"


// Sets default values
AFlecsZombieHorde::AFlecsZombieHorde()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFlecsZombieHorde::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFlecsZombieHorde::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AFlecsZombieHorde::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AFlecsZombieHorde::AddEntityToSquad(flecs::entity Entity, int32 InstanceIndex)
{
	SquadEntities.Add(Entity);
	InstanceIndices.Add(InstanceIndex);
}