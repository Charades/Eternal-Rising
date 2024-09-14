// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ER : ModuleRules
{
	public ER(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] { "FlecsLibrary", "SteamLibrary", "Slate", "SlateCore", "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemSteam", "Steamworks" });
		
		AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
	}
}
