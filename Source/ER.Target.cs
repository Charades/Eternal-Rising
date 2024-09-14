// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ERTarget : TargetRules
{
	public ERTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.AddRange( new string[] {"ER", "SteamLibrary", "FlecsLibrary" } );
	}
	
	internal static void ApplySharedTargetSettings(TargetRules Target)
	{
		//Target.bUseLoggingInShipping = true;
		Target.GlobalDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=480");
		Target.GlobalDefinitions.Add("UE_PROJECT_STEAMPRODUCTNAME=\"spacewar\"");
		Target.GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDIR=\"spacewar\"");
		Target.GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDESC=\"Spacewar\"");
	}
}
