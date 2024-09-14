using UnrealBuildTool;

public class SteamLibrary : ModuleRules
{
	public SteamLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemSteam", "Steamworks", "ER"
		});

		AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
		
		//The path for the header files
		PublicIncludePaths.AddRange(new string[] {"SteamLibrary/Public"});
		//The path for the source files
		PrivateIncludePaths.AddRange(new string[] { "SteamLibrary/Private" });
	}
}