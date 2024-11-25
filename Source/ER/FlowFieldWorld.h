// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowFieldCPP.h"
#include "FlowFieldWorld.generated.h"

UCLASS()
class ER_API AFlowFieldWorld : public AFlowFieldCPP
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlowFieldWorld();

	UPROPERTY(BlueprintReadWrite)
	TArray<FS_Cell> GridCells;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
