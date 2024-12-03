// Copyright 2022, Ludvig Köhn, All Rights Reserved.


#include "FlowFieldCPP.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "..\Public\FlowFieldCPP.h"


// Sets default values
AFlowFieldCPP::AFlowFieldCPP()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// Set InstancedStaticMeshComponent, no collision
	InstancedStaticMeshComponentCell = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ALLGRIDS"));
	InstancedStaticMeshComponentCell->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	InstancedStaticMeshComponentCell->SetGenerateOverlapEvents(false);
	RootComponent = InstancedStaticMeshComponentCell;

	InstancedStaticMeshComponentArrow = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ALLARROWS"));
	InstancedStaticMeshComponentArrow->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	InstancedStaticMeshComponentArrow->SetGenerateOverlapEvents(false);
	InstancedStaticMeshComponentArrow->SetupAttachment(RootComponent);

	InstancedStaticMeshComponentObstacle = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ALLOBSTACLES"));
	InstancedStaticMeshComponentObstacle->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	InstancedStaticMeshComponentObstacle->SetGenerateOverlapEvents(false);
	InstancedStaticMeshComponentObstacle->SetupAttachment(RootComponent);

	box = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	box->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	box->SetGenerateOverlapEvents(false);
	box->SetupAttachment(RootComponent);

	cellSize = float(100);
	cellRadius = float(cellSize / 2);

	gridSize = FVector2D(1000, 1000);

	xAmount = int(0);
	yAmount = int(0);

	showGridInGame = true;
	showArrowsInGame = true;
	goalPosition = FVector2D(-1, -1);
	traceForObstacles = true;
	traceHeight = 300;
	alignToGround = false;
	maxWalkableGroundAngle = 44;
	obstacleType.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));

}

// Called when the game starts or when spawned
void AFlowFieldCPP::BeginPlay()
{
	Super::BeginPlay();

	xAmount = int(gridSize.X / cellSize);
	yAmount = int(gridSize.Y / cellSize);


	//FlushPersistentDebugLines(GetWorld());

	//GenerateFlowField();

	/*for (int i = 0; i < AllCells.Num(); i++)
	{
		FS_Cell newCell;

		newCell.cellBestCost = AllCells[i].bestCost;
		newCell.cellCost = AllCells[i].cost;
		newCell.cellGridPos = AllCells[i].pos;
		newCell.cellWorldPos = AllCells[i].worldPos;
		newCell.cellIndex = i;
		newCell.cellDir = AllCells[i].dir;

		AllCellsBP.Add(newCell);

		CellClass curCell = AllCells[i];

		//DrawDebugLine(GetWorld(), curCell.worldPos + (0, 0, 1),curCell.worldPos+curCell.dir*cellRadius, FColor(255,255,255),true,-1,0,5);
	}*/
}

// Called every frame
void AFlowFieldCPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AFlowFieldCPP::GenerateFlowField(const TArray<FS_Cell> GridCells, const FVector goal, TMap<FVector2D, FVector>& directionMap, FVector& goalWorldPos)
{

	TArray<CellClass> _gridCells;

	for (int i = 0; i < GridCells.Num(); i++)
	{
		FS_Cell oldCell = GridCells[i];
		CellClass newCell;
		newCell.cost = oldCell.cellCost;
		newCell.bestCost = oldCell.cellBestCost;
		newCell.pos = oldCell.cellGridPos;
		newCell.worldPos = oldCell.cellWorldPos;
		newCell.index = oldCell.cellIndex;
		newCell.dir = oldCell.cellDir;
		newCell.normal = oldCell.cellNormal;

		_gridCells.Add(newCell);
	}

	AllCells = _gridCells;

	//Finds the closest 2D vector to the input world position
	FVector relativePos = FVector(GetActorLocation() - goal + FVector(cellRadius, cellRadius, 0));
	FVector2D goal2DPos = FVector2D(UKismetMathLibrary::Abs(UKismetMathLibrary::Round(relativePos.X / cellSize)), UKismetMathLibrary::Abs(UKismetMathLibrary::Round(relativePos.Y / cellSize)));

	//Set the output goal world position
	goalWorldPos = FVector(GetActorLocation().X + goal2DPos.X * cellSize + cellRadius, GetActorLocation().Y + goal2DPos.Y * cellSize + cellRadius, goal.Z);

	//Set the goal position to the function input
	goalPosition = goal2DPos;

	//Creates the flow field
	TArray<CellClass> newGridCells;
	newGridCells = CreateFlowField(CreateIntegrationField(goal2DPos, _gridCells));

	TMap<FVector2D, FVector> _dirMap;

	//DirMap.Empty();

	for (int i = 0; i < newGridCells.Num(); i++)
	{
		_dirMap.Add(_gridCells[i].pos, newGridCells[i].dir);
	}
	directionMap = _dirMap;
}

//Create Grid
void AFlowFieldCPP::CreateGrid(TArray<FS_Cell>& GridCells)
{
	AllCellsBP.Empty();

	//Clear all instances
	InstancedStaticMeshComponentCell->ClearInstances();
	InstancedStaticMeshComponentObstacle->ClearInstances();

	//Get loop amount
	xAmount = int(gridSize.X / cellSize);
	yAmount = int(gridSize.Y / cellSize);

	float angleNorm = FMath::DegreesToRadians(maxWalkableGroundAngle);

	//Array of actors for trace to ignore
	TArray<AActor*> IgnoreActors;
	//Add self to ignore actors
	IgnoreActors.Add(GetOwner());

	//Create hit result variable
	FHitResult HitArray;

	//Clear the cell array
	AllCells.Empty();

	//Get actor location and make it a local variable
	FVector actorLoc = GetActorLocation();

	cellRadius = float(cellSize / 2);

	int indx = 0;

	for (int x = 0; x < xAmount; x++)
	{
		for (int y = 0; y < yAmount; y++)
		{
			//Create local new cell struct
			FS_Cell newCell_Struct;

			FVector wPos = FVector(x * cellSize + actorLoc.X + cellRadius, y * cellSize + actorLoc.Y + cellRadius, actorLoc.Z);
			FRotator wRot = FRotator(0, 0, 0);

			if (alignToGround)
			{
				bool hitGround = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), wPos + FVector(0, 0, traceHeight), FVector(wPos.X, wPos.Y, wPos.Z - traceHeight * 2), groundObjectType, true, IgnoreActors,
					EDrawDebugTrace::None, HitArray, true, FLinearColor::Gray, FLinearColor::Blue, 2);

				if (hitGround)
				{
					wPos = HitArray.ImpactPoint;
					wRot = FRotationMatrix::MakeFromZ(HitArray.ImpactNormal).Rotator();
					newCell_Struct.cellNormal = HitArray.ImpactNormal;
				}
			}

			//Set new cell struct pos and world pos
			newCell_Struct.cellGridPos = FVector2D(x, y);
			newCell_Struct.cellWorldPos = wPos;

			float cellAngle = UKismetMathLibrary::Acos(FVector::DotProduct(HitArray.Normal, FVector(0, 0, 1)));

			if (cellAngle > angleNorm && alignToGround)
			{
				newCell_Struct.cellCost = 255;
			}
			else
			{
				if (traceForObstacles)
				{
					bool hit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), wPos, wPos, cellRadius, obstacleType, false, IgnoreActors,
						EDrawDebugTrace::None, HitArray, true, FLinearColor::Gray, FLinearColor::Blue, 2);
					if (hit)
					{
						newCell_Struct.cellCost = 255;
					}
					else
					{
						newCell_Struct.cellCost = 1;
					}
				}
				else
				{
					newCell_Struct.cellCost = 1;
				}
			}




			newCell_Struct.cellBestCost = 65535;
			newCell_Struct.cellIndex = indx;

			GridCells.Add(newCell_Struct);
			AllCellsBP.Add(newCell_Struct);


			//Add an instance for every cell
			if (showGridInGame == true)
			{
				FTransform trans = FTransform(wRot, wPos, FVector(cellSize / 100, cellSize / 100, 1));

				if (newCell_Struct.cellCost == 255)
				{
					InstancedStaticMeshComponentObstacle->AddInstance(trans, true);
				}
				else
				{
					InstancedStaticMeshComponentCell->AddInstance(trans, true);
				}
			}



			//Add to index
			indx++;
		}
	}

}

void AFlowFieldCPP::DebugGrid()
{
	//Get loop amount
	xAmount = int(gridSize.X / cellSize);
	yAmount = int(gridSize.Y / cellSize);

	float angleNorm;

	if (alignToGround)
	{
		angleNorm = FMath::DegreesToRadians(maxWalkableGroundAngle);
	}
	else
	{
		angleNorm = 2;
	}
	//Set the cell radius
	cellRadius = float(cellSize / 2);

	//Clear all instances
	InstancedStaticMeshComponentCell->ClearInstances();
	InstancedStaticMeshComponentObstacle->ClearInstances();

	int indx = 0;

	//Create hit result variable
	FHitResult HitArray;

	//Array of actors for trace to ignore
	TArray<AActor*> IgnoreActors;
	//Add self to ignore actors
	IgnoreActors.Add(GetOwner());

	//Get Actor location
	FVector actorLoc = GetActorLocation();

	for (int x = 0; x < xAmount; x++)
	{
		for (int y = 0; y < yAmount; y++)
		{
			FVector wPos = FVector(x * cellSize + actorLoc.X + cellRadius, y * cellSize + actorLoc.Y + cellRadius, actorLoc.Z);
			FRotator wRot = FRotator(0, 0, 0);
			FVector newWPos = wPos;

			if (alignToGround)
			{
				bool hitGround = UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), wPos + FVector(0, 0, traceHeight), FVector(wPos.X, wPos.Y, wPos.Z - traceHeight * 2), groundObjectType, true, IgnoreActors,
					EDrawDebugTrace::None, HitArray, true, FLinearColor::Gray, FLinearColor::Blue, 2);

				if (hitGround)
				{
					newWPos = HitArray.ImpactPoint;
					wRot = FRotationMatrix::MakeFromZ(HitArray.ImpactNormal).Rotator();

				}
			}

			FTransform trans = FTransform(wRot, newWPos, FVector(cellSize / 100, cellSize / 100, 1));

			bool tooSteep = false;
			bool hit;

			if (UKismetMathLibrary::Acos(FVector::DotProduct(HitArray.Normal, FVector(0, 0, 1))) > angleNorm)
			{
				tooSteep = true;
			}

			if (traceForObstacles)
			{
				hit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), newWPos, newWPos, cellRadius, obstacleType, false, IgnoreActors,
					EDrawDebugTrace::None, HitArray, true, FLinearColor::Gray, FLinearColor::Blue, 5);
			}
			else
			{
				hit = false;
			}


			if (hit || tooSteep)
			{
				InstancedStaticMeshComponentObstacle->AddInstance(trans, true);
			}
			else
			{
				InstancedStaticMeshComponentCell->AddInstance(trans, true);
			}

			//Add to index
			indx++;
		}
	}
}

bool AFlowFieldCPP::CheckIfNeigbourClose(const FVector2D currentPos, const FVector2D neighbourPos)
{
	if (currentPos[0] - 1 <= neighbourPos[0] && neighbourPos[0] <= currentPos[0] + 1 && currentPos[1] - 1 <= neighbourPos[1] && neighbourPos[1] <= currentPos[1] + 1) {
		return true;
	}
	else {
		return false;
	}
}

//Create Integration Field
TArray<CellClass> AFlowFieldCPP::CreateIntegrationField(const FVector2D DestinationPos, const TArray<CellClass> GridCells)
{
	TArray<CellClass> _gridCells = GridCells;
	int destIndex = 0;

	for (int i = 0; i < _gridCells.Num(); i++)
	{
		if (DestinationPos == _gridCells[i].pos)
		{
			destIndex = _gridCells[i].index;
			_gridCells[i].bestCost = 0;
			_gridCells[i].cost = 0;
			break;
		}
	}

	CellClass destCell = _gridCells[destIndex];

	TArray<CellClass> gridCellsToCheck;

	int ind = 0;

	gridCellsToCheck.Add(destCell);

	while (gridCellsToCheck.Num() > 0) {

		CellClass curCell = gridCellsToCheck[0];

		int nIndex = 0;
		int currentIndex = curCell.index;

		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
			{
				nIndex = currentIndex + yAmount;
			}
			if (i == 1)
			{
				nIndex = currentIndex + 1;
			}
			if (i == 2)
			{
				nIndex = currentIndex - yAmount;
			}
			if (i == 3)
			{
				nIndex = currentIndex - 1;
			}

			if (nIndex < 0 || nIndex > _gridCells.Num() - 1) { continue; }

			CellClass neighbourCell = _gridCells[nIndex];

			if (neighbourCell.cost == 255) { continue; }
			if (FMath::Abs(curCell.worldPos.Z - neighbourCell.worldPos.Z) > cellSize) { continue; }

			//UE_LOG(LogTemp, Warning, TEXT("Lenght: %f"), FVector::DotProduct(FVector(curCell.normal.X,curCell.normal.Y,0), FVector(neighbourCell.normal.X, neighbourCell.normal.Y, 0)));

			if (CheckIfNeigbourClose(curCell.pos, neighbourCell.pos) == false) { continue; }
			if (neighbourCell.cost + curCell.bestCost < neighbourCell.bestCost)
			{
				neighbourCell.bestCost = int(neighbourCell.cost + curCell.bestCost);
				_gridCells[neighbourCell.index].bestCost = int(neighbourCell.cost + curCell.bestCost);
				gridCellsToCheck.Add(_gridCells[neighbourCell.index]);
				ind++;

			}
		}
		gridCellsToCheck.RemoveAt(0);
	}
	return _gridCells;
}

TArray<CellClass> AFlowFieldCPP::CreateFlowField(const TArray<CellClass> GridCells)
{
	TArray<CellClass> _gridCells = GridCells;

	InstancedStaticMeshComponentArrow->ClearInstances();

	for (int j = 0; j < _gridCells.Num(); j++)
	{

		CellClass curCell = _gridCells[j];

		int _bestCost = curCell.bestCost;

		float bestDot = 0;

		int currentIndex = j;

		int nIndex;

		for (int i = 0; i < 8; i++)
		{
			if (i == 0)
			{
				nIndex = currentIndex + yAmount;
			}
			if (i == 1)
			{
				nIndex = currentIndex + 1;
			}
			if (i == 2)
			{
				nIndex = currentIndex - yAmount;
			}
			if (i == 3)
			{
				nIndex = currentIndex - 1;
			}
			if (i == 4)
			{
				nIndex = currentIndex + yAmount + 1;
			}
			if (i == 5)
			{
				nIndex = currentIndex - yAmount + 1;
			}
			if (i == 6)
			{
				nIndex = currentIndex - yAmount - 1;
			}
			if (i == 7)
			{
				nIndex = currentIndex + yAmount - 1;
			}

			if (nIndex < 0 || nIndex > _gridCells.Num() - 1) { continue; }

			CellClass neighbourCell = _gridCells[nIndex];


			//UE_LOG(LogTemp, Warning, TEXT("Lenght: %f"), UKismetMathLibrary::Abs((_gridCells[nIndex].worldPos.Z - curCell.worldPos.Z)));
			if (CheckIfNeigbourClose(curCell.pos, neighbourCell.pos) == false) { continue; }
			if (IsDiagonalValid(i, currentIndex, nIndex) == false) { continue; }
			if (FMath::Abs(curCell.worldPos.Z - neighbourCell.worldPos.Z) > cellSize) { continue; }

			float vDot = FVector::DotProduct(curCell.normal, neighbourCell.normal);

			//if (vDot < bestDot) { continue; }

			if (neighbourCell.bestCost < _bestCost)
			{
				_bestCost = neighbourCell.bestCost;
				_gridCells[currentIndex].dir = UKismetMathLibrary::GetDirectionUnitVector(curCell.worldPos, neighbourCell.worldPos);
				//bestDot = vDot;
			}

		}
		if (showArrowsInGame && _gridCells[currentIndex].dir.IsZero() == false)
		{
			FRotator newRot = FRotationMatrix::MakeFromX(_gridCells[currentIndex].dir).Rotator();
			InstancedStaticMeshComponentArrow->AddInstance(FTransform(newRot, FVector(curCell.worldPos) + FVector(0, 0, 1), FVector(cellSize / 100)), true);
		}
	}
	return _gridCells;

}

bool AFlowFieldCPP::IsDiagonalValid(const int loopIndex, const int curIndex, const int neighbourIndex)
{
	int maxIndex = AllCells.Num();

	if (loopIndex == 4 || loopIndex == 5)
	{
		if (curIndex + 1 < maxIndex && neighbourIndex - 1 > -1)
		{
			if (AllCells[curIndex + 1].cost != 255 && AllCells[neighbourIndex - 1].cost != 255)
			{
				return true;
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}
	}
	else if (loopIndex == 6 || loopIndex == 7)
	{
		if (curIndex - 1 > -1 && neighbourIndex + 1 < maxIndex)
		{
			if (AllCells[curIndex - 1].cost != 255 && AllCells[neighbourIndex + 1].cost != 255)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}

}


void AFlowFieldCPP::OnConstruction(const FTransform& scale)
{
	Super::OnConstruction(scale);

	box->SetBoxExtent(FVector(gridSize.X / 2, gridSize.Y / 2, traceHeight / 2));
	box->SetRelativeLocation(FVector(gridSize.X / 2, gridSize.Y / 2, traceHeight / 2));

	//CreateGrid(traceForObstaclesRealTime);
}





