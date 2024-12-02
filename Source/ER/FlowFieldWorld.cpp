// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldWorld.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AFlowFieldWorld::AFlowFieldWorld(): bIsInitialized(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AFlowFieldWorld::BeginPlay()
{
	Super::BeginPlay();
	
	CreateGrid(GridCells);
}

// Called every frame
void AFlowFieldWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFlowFieldWorld::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFlowFieldWorld, bIsInitialized);
}