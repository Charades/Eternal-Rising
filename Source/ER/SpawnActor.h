// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "SpawnActor.generated.h"

UCLASS()
class ER_API ASpawnActor : public AActor
{
	GENERATED_BODY()

bool bMenuToggled;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* OrbMesh;

	// Material to apply on the sphere
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
	UMaterialInterface* MatSelected;

	// Initial material of the sphere
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* MatDefault;
	
public:
	// Called every frame
	ASpawnActor();
	bool ShowSpawnMenu(bool bSetToggle);
	void SpawnPoints(float Radius, int32 NumberOfPoints);
	virtual void Tick(float DeltaTime) override;
};
