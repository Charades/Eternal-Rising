#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "flecs.h"
#include "FlecsSubsystem.generated.h"

struct FlecsTransform
{
	FTransform Value;
};

struct FlecsZombie
{
	FVector Position;
};

struct FlecsISMIndex
{
	int Value;
};

struct FlecsIsmRef
{
	UInstancedStaticMeshComponent* Value;
};

struct Horde {};

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

	UPROPERTY(EditAnywhere)
	UInstancedStaticMeshComponent* ZombieRenderer = nullptr;

	UFUNCTION(BlueprintCallable, Category="FLECS")
	void InitFlecs(UStaticMesh* InMesh);

	UFUNCTION(BlueprintCallable, Category="FLECS")
	FFlecsEntityHandle SpawnZombieEntity(FVector location, FRotator rotation);

protected:
	FTickerDelegate OnTickDelegate;
	FTSTicker::FDelegateHandle OnTickHandle;
	flecs::world* ECSWorld = nullptr;

private:
	bool Tick(float DeltaTime);
};
