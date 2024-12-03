// Copyright 2022, Ludvig Köhn, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class FLOWFIELDPATHFINDING_API CellClass
{
public:
	CellClass();

	FVector2D pos;
	FVector worldPos;
	BYTE cost;
	USHORT bestCost;
	bool checked;
	int index;
	FVector dir;
	FVector normal;

	~CellClass();
};
