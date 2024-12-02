#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "AnimToTextureDataAsset.h"
#include "ZombieMeshStruct.generated.h"

USTRUCT(BlueprintType)
struct FZombieMeshStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimToTextureDataAsset* DataAsset;
};