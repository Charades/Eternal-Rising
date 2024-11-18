// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer),
                                                                   ServerBrowserButton(nullptr),
                                                                   ExitGameButton(nullptr),
                                                                   ServerBrowser(nullptr)
{
	static ConstructorHelpers::FClassFinder<UServerBrowser> ServerBrowserWidgetFinder(TEXT("/Game/Blueprints/UI/ServerBrowser"));
	if (ServerBrowserWidgetFinder.Succeeded())
	{
		ServerBrowserWidget = ServerBrowserWidgetFinder.Class;
		UE_LOG(LogTemp, Log, TEXT("Found Server Browser Widget!"));
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> SettingsMenuWidgetFinder(TEXT("/Game/Blueprints/UI/SettingsMenu"));
	if (SettingsMenuWidgetFinder.Succeeded())
	{
		SettingsMenuWidget = SettingsMenuWidgetFinder.Class;
		UE_LOG(LogTemp, Log, TEXT("Found Settings Menu Widget!"));
	}
}

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateSteamInfo();
	ServerBrowserButton = Cast<UButton>(GetWidgetFromName(TEXT("ServerBrowserButton")));
	ExitGameButton = Cast<UButton>(GetWidgetFromName(TEXT("ExitGameButton")));
	MusicToggleButton = Cast<UButton>(GetWidgetFromName(TEXT("MusicToggleButton")));
	
	if (ServerBrowserButton)
	{
		ServerBrowserButton->OnClicked.AddDynamic(this, &UMainMenu::OnServerBrowserButtonClicked);
	}

	if (SettingsMenuButton)
	{
		SettingsMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OnSettingsMenuButtonClicked);
	}

	if (SteamProfileButton)
	{
		SteamProfileButton->OnClicked.AddDynamic(this, &UMainMenu::OnSteamProfileButtonClicked);
	}
	
	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.AddDynamic(this, &UMainMenu::OnExitGameButtonClicked);
	}

	if (MusicToggleButton)
	{
		MusicToggleButton->OnClicked.AddDynamic(this, &UMainMenu::StopBackgroundMusic);
	}
	
	if (BackgroundMusic)
	{
		StartBackgroundMusic(BackgroundMusic);
	}
}

void UMainMenu::NativeDestruct()
{
	Super::NativeDestruct();
    
	// Clean up the audio component if it exists
	if (AudioComponent)
	{
		AudioComponent->Stop();
		AudioComponent->DestroyComponent();
		AudioComponent = nullptr;
	}
}

void UMainMenu::UpdateSteamInfo()
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

void UMainMenu::GetSteamAvatar()
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

void UMainMenu::RemoveFromParent()
{
	if (ServerBrowser && ServerBrowser->IsInViewport())
	{
		ServerBrowser->RemoveFromParent();
		ServerBrowser = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Server Browser hidden."));
	}
	
	Super::RemoveFromParent();
}

void UMainMenu::OnSteamProfileButtonClicked()
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

void UMainMenu::OnServerBrowserButtonClicked()
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

void UMainMenu::OnSettingsMenuButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Settings Menu clicked!"));
	}

	if (SettingsMenuWidget)
	{
		if (!SettingsMenu)
		{
			SettingsMenu = CreateWidget<UUserWidget>(this, SettingsMenuWidget);
		}

		if (SettingsMenu)
		{
			SettingsMenu->AddToViewport();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingsMenuWidget is null."));
	}
}

void UMainMenu::OnExitGameButtonClicked()
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;

	if (PlayerController)
	{
		UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, true);
	}
}

void UMainMenu::StartBackgroundMusic(USoundCue* MusicToPlay)
{
	// Stop any existing music first
	StopBackgroundMusic();
    
	if (MusicToPlay)
	{
		// Create and start the background music
		AudioComponent = UGameplayStatics::SpawnSound2D(this, MusicToPlay);
        
		if (AudioComponent)
		{
			AudioComponent->bAutoDestroy = false;
			AudioComponent->bIsUISound = true;
		}
	}
}

void UMainMenu::StopBackgroundMusic()
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}