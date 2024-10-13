#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FlecsZombieHorde.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FlecsAIController.h"
#include "flecs.h"
#include "FlecsSubsystem.generated.h"

struct FlecsTransform
{
	FTransform Value;
};

struct FlecsLastTraceTime {
	float LastTraceTime = 0.0f;
};

struct FlecsZombie
{
	float Health;
};

struct Horde {};

struct FlecsISMIndex
{
	int Value;
};

struct FlecsTargetLocation
{
	FVector Value;
};

struct FlecsIsmRef
{
	UInstancedStaticMeshComponent* Value;
};

struct FlecsHordeRef
{
	AFlecsZombieHorde* Value;
};

USTRUCT(BlueprintType)
struct FFlecsEntityHandle
{
	GENERATED_USTRUCT_BODY()
	FFlecsEntityHandle()  {}
	FFlecsEntityHandle(int inId)
	{
		FlecsEntityId = inId;
	}
	UPROPERTY(BlueprintReadWrite)
	int FlecsEntityId;
};

UCLASS()
class ER_API UFlecsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	flecs::world* GetEcsWorld() const;

	//UPROPERTY(EditAnywhere)
	//UInstancedStaticMeshComponent* ZombieRenderer = nullptr;

	UFUNCTION(BlueprintCallable, Category="FLECS")
	void InitFlecs(UStaticMesh* InMesh);

	UFUNCTION(BlueprintCallable, Category="FLECS")
	void SpawnZombieEntity(FVector Location, FRotator Rotation, UInstancedStaticMeshComponent* ZombieRendererInst);
	void SpawnZombieHorde(FVector SpawnLocation, float Radius, int32 NumEntities);
	
	TWeakObjectPtr<UStaticMesh> DefaultMesh;

	UFUNCTION(BlueprintImplementableEvent, Category = "AI")
	void OnHordeSpawned(AFlecsZombieHorde* Horde);
	
	UFUNCTION(BlueprintPure, Category = "AI")
	TArray<AFlecsZombieHorde*>& Hordes() { return AgentInstances; }

protected:
	FTickerDelegate OnTickDelegate;
	FTSTicker::FDelegateHandle OnTickHandle;
	flecs::world* ECSWorld = nullptr;
	int32 CurrentAgentIndex = -1;

	UPROPERTY(Transient)
	TArray<AFlecsZombieHorde*> AgentInstances;

	UPROPERTY(Category = Spawn, EditDefaultsOnly)
	TSubclassOf<AFlecsZombieHorde> HordeBP;

private:
	bool Tick(float DeltaTime);
};
