// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapeMenu.h"
#include "Components/Button.h"
#include "ER/ClientGameInstance.h"

UEscapeMenu::UEscapeMenu(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UServerBrowser> ServerBrowserWidgetFinder(
	TEXT("/Game/Blueprints/UI/ServerBrowser"));
	if (ServerBrowserWidgetFinder.Succeeded())
	{
		ServerBrowserWidget = ServerBrowserWidgetFinder.Class;
		UE_LOG(LogTemp, Log, TEXT("Found Server Browser Widget!"));
	}
}

void UEscapeMenu::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateSteamInfo();
	ServerBrowserButton = Cast<UButton>(GetWidgetFromName(TEXT("ServerBrowserButton")));
	ExitGameButton = Cast<UButton>(GetWidgetFromName(TEXT("ExitGameButton")));
	
	if (ServerBrowserButton)
	{
		ServerBrowserButton->OnClicked.AddDynamic(this, &UEscapeMenu::OnServerBrowserButtonClicked);
	}
	
	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.AddDynamic(this, &UEscapeMenu::OnExitGameButtonClicked);
	}

	if (SteamProfileButton)
	{
		SteamProfileButton->OnClicked.AddDynamic(this, &UEscapeMenu::OnSteamProfileButtonClicked);
	}
}

void UEscapeMenu::RemoveFromParent()
{
	if (ServerBrowser && ServerBrowser->IsInViewport())
	{
		ServerBrowser->RemoveFromParent();
		ServerBrowser = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Server Browser hidden."));
	}
	
	Super::RemoveFromParent();
}

void UEscapeMenu::OnServerBrowserButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Server button clicked!"));
	}

	if (ServerBrowserWidget)
	{
		if (!ServerBrowser)
		{
			ServerBrowser = CreateWidget<UServerBrowser>(this, ServerBrowserWidget);
		}

		if (ServerBrowser)
		{
			if (ServerBrowser->IsInViewport())
			{
				ServerBrowser->RemoveFromParent();
				ServerBrowser = nullptr;
				UE_LOG(LogTemp, Log, TEXT("Server Browser hidden."));
			}
			else
			{
				ServerBrowser->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("Server Browser displayed."));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerBrowserWidget is null."));
	}
}

void UEscapeMenu::OnExitGameButtonClicked()
{
	// UWorld* World = GetWorld();
	// APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	//
	// if (PlayerController)
	// {
	// 	UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, true);
	// }

	UClientGameInstance* GameInstance = Cast<UClientGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->ReturnToMainMenu();
	}
}

void UEscapeMenu::UpdateSteamInfo()
{
	if (!SteamAPI_Init()) return;

	// Get the Steam user ID (local player)
	CSteamID SteamID = SteamUser()->GetSteamID();

	// Get the Steam account name
	FString AccountName = UTF8_TO_TCHAR(SteamFriends()->GetPersonaName());
	if (AccountNameText)
	{
		AccountNameText->SetText(FText::FromString(AccountName));
	}
	
	GetSteamAvatar();
}

void UEscapeMenu::GetSteamAvatar()
{
	// Get Steam avatar handle
	int32 AvatarHandle = SteamFriends()->GetMediumFriendAvatar(SteamUser()->GetSteamID());
    
	if (AvatarHandle > 0)
	{
		uint32 Width, Height;
		SteamUtils()->GetImageSize(AvatarHandle, &Width, &Height);
        
		if (Width > 0 && Height > 0)
		{
			// Create buffer for the avatar data
			TArray<uint8> AvatarRGBA;
			AvatarRGBA.SetNum(Width * Height * 4);
            
			// Get the avatar RGBA data
			if (SteamUtils()->GetImageRGBA(AvatarHandle, AvatarRGBA.GetData(), AvatarRGBA.Num()))
			{
				// Create a texture from the RGBA data
				UTexture2D* AvatarTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
                
				if (AvatarTexture)
				{
					// Lock the texture for writing
					void* TextureData = AvatarTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
					FMemory::Memcpy(TextureData, AvatarRGBA.GetData(), AvatarRGBA.Num());
					AvatarTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
                    
					// Update the texture
					AvatarTexture->UpdateResource();
                    
					// Set the texture to your image widget
					if (AvatarImage)
					{
						AvatarImage->SetBrushFromTexture(AvatarTexture);
					}
				}
			}
		}
	}
}

void UEscapeMenu::OnSteamProfileButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Steam profile button clicked!"));
	}

	if (SteamAPI_Init())
	{
		// Open the Steam Overlay to the "Friends" section
		SteamFriends()->ActivateGameOverlay("Friends");
		UE_LOG(LogTemp, Log, TEXT("Steam Overlay opened successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Steam API not initialized. Unable to open Steam Overlay."));
	}
}

