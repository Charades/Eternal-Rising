// Fill out your copyright notice in the Description page of Project Settings.


#include "FlecsAIController.h"
#include "NavigationSystem.h"
#include "NavFilters/NavigationQueryFilter.h"

// Sets default values
AFlecsAIController::AFlecsAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFlecsAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFlecsAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

