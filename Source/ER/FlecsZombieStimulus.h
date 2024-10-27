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
	AFlecsZombieStimulus() = default;

	UFUNCTION(BlueprintNativeEvent, Category = AI)
	void Consume(UFlecsZombieBoid* Boid, AFlecsZombieHorde* Agent = nullptr);
	void Consume_Implementation(UFlecsZombieBoid* Boid, AFlecsZombieHorde* Agent = nullptr);

	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;

	UPROPERTY(Category = AI, EditAnywhere, BlueprintReadWrite)
	float Radius = 0.0f;
};