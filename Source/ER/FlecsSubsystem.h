#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "flecs.h"
#include "FlecsSubsystem.generated.h"

UCLASS()
class ER_API UFlecsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	bool Tick(float DeltaTime);
};
