// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimToTextureInstancePlaybackHelpers.h"
#include "GameFramework/Pawn.h"
#include "FlowFieldMovement.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ZombieMeshStruct.h"
#include "FlecsZombieBoid.generated.h"

class AFlecsZombieBoid;

UCLASS(BlueprintType, Blueprintable)
class ER_API AFlecsZombieBoid : public APawn
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	AFlecsZombieBoid(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	UFlowFieldMovement* FlowFieldMovement;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Default")
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	UFloatingPawnMovement* FloatingPawnMovement;
	
	UFUNCTION()
	bool SetAnimation(int index);

	UFUNCTION()
	void PerformAttack(AActor* Actor);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ApplyDelayedDamage();
	
	FTimerHandle DamageTimerHandle;

	UPROPERTY()
	AActor* CurrentTarget;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	FZombieMeshStruct* Row;

	TArray<FMatrix> _Matrices;

	void BuildAutoPlayData(UAnimToTextureDataAsset* InDA, TArray<FAnimToTextureAutoPlayData>& AutoPlayDataArray);

	void BuildMatrices(TArray<FMatrix>& Matrices);

	TArray<FAnimToTextureAutoPlayData> _AutoPlayDataArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Default")
	UCapsuleComponent* CollisionComponent;

	bool bAutoPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 2.0f; // Time in seconds between attacks

	UPROPERTY()
	float LastAttackTime = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageDelay = 0.5f; // Time to wait before applying damage

private:
	UPROPERTY(EditAnywhere)
	UDataTable* DataTable;  // Set this in the editor
	
	void InitializeMeshInstances();
};