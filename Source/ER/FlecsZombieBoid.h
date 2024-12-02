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
	AFlecsZombieBoid(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	UFlowFieldMovement* FlowFieldMovement;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Default")
	UInstancedStaticMeshComponent* InstancedStaticMeshComponent;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	UFloatingPawnMovement* FloatingPawnMovement;
	
	UFUNCTION()
	void SetAnimation(int index);
	void PerformAttack(AActor* Actor);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

private:
	UPROPERTY(EditAnywhere)
	UDataTable* DataTable;  // Set this in the editor
	
	void InitializeMeshInstances();
};