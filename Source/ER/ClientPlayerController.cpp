// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputData.h"
#include "SpawnActor.h"
#include "Blueprint/UserWidget.h"
#include "UI/EscapeMenu.h"
#include "Steam/steam_api.h"
#include "Steam/isteamnetworkingsockets.h"
#include "Steam/isteamnetworkingutils.h"

AClientPlayerController::AClientPlayerController()
{
	// Find the InputMappingContext asset in the content browser
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextFinder(
		TEXT("/Game/GameContent/Blueprints/IMC_Default"));
	if (MappingContextFinder.Succeeded())
	{
		DefaultMappingContext = MappingContextFinder.Object;
		UE_LOG(LogTemp, Warning, TEXT("Found Mapping Context!"));
	}

	// Find the InputActionDataAsset in the content browser
	static ConstructorHelpers::FObjectFinder<UInputData> InputDataAssetFinder(
		TEXT("/Game/GameContent/Blueprints/DA_InputActions"));
	if (InputDataAssetFinder.Succeeded())
	{
		InputData = InputDataAssetFinder.Object;
		UE_LOG(LogTemp, Warning, TEXT("Found Input Actions!"));
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> EscapeMenuWidgetFinder(
		TEXT("/Game/GameContent/Blueprints/EscapeMenu"));
	if (EscapeMenuWidgetFinder.Succeeded())
	{
		EscapeMenuWidget = EscapeMenuWidgetFinder.Class;
		UE_LOG(LogTemp, Warning, TEXT("Found Escape Menu Widget!"));
	}
}

void AClientPlayerController::CallRequestServerList()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			ClientNetworkSubsystem->RequestServerList();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ClientNetworkSubsystem is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance is null"));
	}
}

void AClientPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Ensure the Enhanced Input Local Player Subsystem is initialized
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer)) {
			// Add the default mapping context
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
        	UE_LOG(LogTemp, Log, TEXT("Default mapping context added."));
		}
        else
        {
        	UE_LOG(LogTemp, Warning, TEXT("Failed to get EnhancedInputLocalPlayerSubsystem."));
        }
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController not found."));
	}
}

void AClientPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Close the connection to the server if it's still open
	if (ServerConnection != k_HSteamNetConnection_Invalid)
	{
		SteamNetworkingSockets()->CloseConnection(ServerConnection, 0, "Client Disconnecting", true);
	}

	Super::EndPlay(EndPlayReason);
}

void AClientPlayerController::ConnectToServer(const FString& IPAddress, int32 Port)
{
	if (IPAddress.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Server address is empty."));
		return;
	}

	SteamNetworkingIPAddr ServerAddress;
	ServerAddress.Clear();
	ServerAddress.ParseString(TCHAR_TO_ANSI(*IPAddress));

	ServerAddress.m_port = Port;
	ServerConnection = SteamNetworkingSockets()->ConnectByIPAddress(ServerAddress, 0, nullptr);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Connecting to server at %s:%d"), *IPAddress, Port));
}

void AClientPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Ensure InputComponent is valid
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InputData) {
			if (InputData->ShowEscapeMenu)
			{
				EnhancedInputComponent->BindAction(InputData->ShowEscapeMenu, ETriggerEvent::Completed, this, &AClientPlayerController::OnShowEscapeMenu);
				UE_LOG(LogTemp, Log, TEXT("Escape Menu action bound."));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ShowEscapeMenu is null."));
			}

			if (InputData->LeftMouseClick)
			{
				EnhancedInputComponent->BindAction(InputData->LeftMouseClick, ETriggerEvent::Completed, this, &AClientPlayerController::LeftMouseClick);
				UE_LOG(LogTemp, Log, TEXT("Left Mouse Click action bound."));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("LeftMouseClick is null."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InputData is null."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found."));
	}
}

void AClientPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetShowMouseCursor(true);
	
	// Handles Steam callbacks on the client
	SteamAPI_RunCallbacks();
}

void AClientPlayerController::OnShowEscapeMenu(const FInputActionValue& Value)
{
	if (EscapeMenuWidget)
	{
		if (!EscapeMenu)
		{
			EscapeMenu = CreateWidget<UEscapeMenu>(this, EscapeMenuWidget);
		}

		if (EscapeMenu)
		{
			if (EscapeMenu->IsInViewport())
			{
				EscapeMenu->RemoveFromParent();
				EscapeMenu = nullptr;
				SetShowMouseCursor(false);
				SetInputMode(FInputModeGameOnly());
			}
			else
			{
				EscapeMenu->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("Escape Menu Widget displayed."));
				SetShowMouseCursor(true);
				SetInputMode(FInputModeGameAndUI());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Escape Menu Widget is null."));
	}
}

void AClientPlayerController::LeftMouseClick(const FInputActionValue& Value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Left Mouse Click Event"));

	ActorHitResults();
}

void AClientPlayerController::ActorHitResults()
{
		FVector WorldLocation, WorldDirection;

		if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			// Perform a line trace from the mouse cursor position
			FVector Start = WorldLocation;
			FVector End = Start + (WorldDirection * 10000.0f);

			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;

			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECC_Visibility,
				CollisionParams
			);

			if (bHit)
			{
				AActor* HitActor = HitResult.GetActor();
				if (HitActor && HitActor->IsA<ASpawnActor>())
				{
					ASpawnActor* Spawner = Cast<ASpawnActor>(HitActor);
					if (Spawner)
					{
						Spawner->ShowSpawnMenu();
					}
				}

				// Just a useful visual that is needed for now
				DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 1.0f);
			}
		}
}