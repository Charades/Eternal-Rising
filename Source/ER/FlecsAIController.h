// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "AI/Navigation/NavQueryFilter.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationData.h"
#include "GameFramework/Character.h"
#include "FlecsAIController.generated.h"

UCLASS()
class ER_API AFlecsAIController : public AAIController
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	AFlecsAIController();
	void MoveToRandomLocation();
	void MoveToTargetLocation(FVector Loc);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FVector CurrentDestination;
	FNavPathSharedPtr CurrentPath;
};
