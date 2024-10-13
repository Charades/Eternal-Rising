// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FlecsZombieStimulus.generated.h"

class AFlecsZombieHorde;

UCLASS(abstract)
class ER_API AFlecsZombieStimulus : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;

	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite)
	float Radius = 0.0f;
};