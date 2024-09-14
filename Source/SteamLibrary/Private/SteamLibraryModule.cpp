#include "SteamLibrary/Public/SteamLibraryModule.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

IMPLEMENT_GAME_MODULE(FSteamLibraryModule, SteamLibrary);

DEFINE_LOG_CATEGORY(SteamLibrary);
 
#define LOCTEXT_NAMESPACE "SteamLibrary"
 
void FSteamLibraryModule::StartupModule()
{
	UE_LOG(SteamLibrary, Warning, TEXT("SteamLibrary module has started!"));
}
 
void FSteamLibraryModule::ShutdownModule()
{
	UE_LOG(SteamLibrary, Warning, TEXT("SteamLibrary module has shut down"));
}
 
#undef LOCTEXT_NAMESPACE