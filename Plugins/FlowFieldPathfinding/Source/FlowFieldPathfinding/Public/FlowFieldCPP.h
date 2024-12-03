// Copyright 2022, Ludvig Köhn, All Rights Reserved.

#pragma once

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CellClass.h"
#include "Kismet/KismetMathLibrary.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowFieldCPP.generated.h"


USTRUCT(BlueprintType) struct FS_Cell
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
		FVector2D cellGridPos;

	UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
		FVector cellWorldPos;

	UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
		int cellCost;

	UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
		int cellBestCost;

	UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
		int cellIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
		FVector cellDir;

	UPROPERTY(BlueprintReadOnly, Category = "Flow Field")
		FVector cellNormal;
};

UCLASS()
class FLOWFIELDPATHFINDING_API AFlowFieldCPP : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Flow Field", meta = (ToolTip = "Generates a flow field based on the input Grid Cells Array and an estimate goal World Position, outputs a Map which contains all the cell directions based on a 2D position and an exact goal world position."))
		void GenerateFlowField(const TArray<FS_Cell> GridCells, const FVector goal, TMap<FVector2D, FVector>& directionMap, FVector& goalWorldPos);

	UFUNCTION(BlueprintCallable, Category = "Flow Field", meta = (ToolTip = "Creates the Flow Field grid based on the flow field parameters, outputs an array that contains all the grid cells."))
		void CreateGrid(TArray<FS_Cell>& GridCells);

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Flow Field", meta = (ToolTip = "Creates a debug grid."))
		void DebugGrid();

	/*Construction script*/
	virtual void OnConstruction(const FTransform& scale) override;

	virtual TArray<CellClass> CreateIntegrationField(const FVector2D DestinationPos, const TArray<CellClass> GridCells);

	virtual TArray<CellClass> CreateFlowField(const TArray<CellClass> GridCells);

	virtual bool IsDiagonalValid(const int loopIndex, const int curIndex, const int neighbourIndex);
	
	float CalculateObstacleCost(const FVector& Position, const TArray<CellClass>& GridCells);
	const float ObstacleAvoidanceWeight = 50.0f; // Adjust this value to control avoidance strength
	const float MaxObstacleInfluence = 300.0f; // Maximum distance obstacles affect pathfinding

	UFUNCTION()
		virtual bool CheckIfNeigbourClose(const FVector2D currentPos, const FVector2D neighbourPos);

	// Sets default values for this actor's properties
	AFlowFieldCPP();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 0, ToolTip = "The scale of the flow field pathfinding boundary in units"))
		FVector2D gridSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 1, ToolTip = "The scale of the cells in units"))
		float cellSize;

	UPROPERTY(meta = (DisplayPriority = 2))
		FVector2D goalPosition;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Flow Field", meta = (DisplayPriority = 3, ToolTip = "If True: Shows the debug grid in-game"))
		bool showGridInGame;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Flow Field", meta = (DisplayPriority = 4, ToolTip = "If True: Shows the flow field arrows in-game"))
		bool showArrowsInGame;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 5, ToolTip = "If True: When creating the grid it will sphere-trace for any mesh that isnt of the given Ground Object Type"))
		bool traceForObstacles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 6, ToolTip = "Type of object the flow field deem as an obstacle"))
		TArray<TEnumAsByte<EObjectTypeQuery>> obstacleType;

	UPROPERTY(BlueprintReadWrite, Category = "Flow Field")
		float cellRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 7, ToolTip = "If True: Trace all cells for any given Ground Object Type and aligns it to the ground"))
		bool alignToGround;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 8, ToolTip = "The height of which the ground trace will start at, in units"))
		float traceHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 9, ToolTip = "The max walkable ground angle of which the ground trace will deem as a walkable cell"))
		float maxWalkableGroundAngle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Flow Field", meta = (DisplayPriority = 10, ToolTip = "Object types that the flow field system will align to"))
		TArray<TEnumAsByte<EObjectTypeQuery>> groundObjectType;

	UPROPERTY()
		int xAmount;

	UPROPERTY()
		int yAmount;

	UPROPERTY()
		int destinationIndex;

	UPROPERTY(BlueprintReadWrite, Category = "Flow Field")
		TMap<FVector2D, FVector> DirMap;

	UPROPERTY(BlueprintReadWrite, Category = "Flow Field")
		TArray<FS_Cell> AllGridCells;

	UPROPERTY()
		UBoxComponent* box;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Debug")
		UInstancedStaticMeshComponent* InstancedStaticMeshComponentArrow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Debug")
		UInstancedStaticMeshComponent* InstancedStaticMeshComponentCell;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Debug")
		UInstancedStaticMeshComponent* InstancedStaticMeshComponentObstacle;



	//UPROPERTY(BlueprintReadWrite)
	TArray<CellClass> AllCells;

	CellClass destinationCell;

	UPROPERTY()
		TArray<FS_Cell> AllCellsBP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field|Obstacle Detection")
	bool bIgnoreCeilingAndFloor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field|Obstacle Detection")
	float MinObstacleHeight = 50.0f;  // Minimum height of an obstacle to be considered

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field|Obstacle Detection")
	float MaxObstacleHeight = 200.0f; 


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
