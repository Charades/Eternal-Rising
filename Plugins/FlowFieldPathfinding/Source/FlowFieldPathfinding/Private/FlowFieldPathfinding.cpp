// Copyright 2022, Ludvig Köhn, All Rights Reserved.

#include "FlowFieldPathfinding.h"

#define LOCTEXT_NAMESPACE "FFlowFieldPathfindingModule"

void FFlowFieldPathfindingModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FFlowFieldPathfindingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFlowFieldPathfindingModule, FlowFieldPathfinding)