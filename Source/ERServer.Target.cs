// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ERServerTarget : TargetRules
{
	public ERServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ERTarget.ApplySharedTargetSettings(this);
		ExtraModuleNames.AddRange( new string[] {"ER", "SteamLibrary", "FlecsLibrary"} );
	}
}
