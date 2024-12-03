// Copyright 2022, Ludvig Köhn, All Rights Reserved.


#include "CellClass.h"

CellClass::CellClass()
{
	pos = FVector2D(0, 0);
	worldPos = FVector(0, 0, 0);
	cost = 0;
	bestCost = 0;
	checked = false;
	index = -1;
	dir = FVector(0, 0, 0);
	normal = FVector(0, 0, 1);
}

CellClass::~CellClass()
{
}
