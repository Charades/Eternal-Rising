// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPlayerController.h"

#include "ClientGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "FlecsZombieHorde.h"
#include "InputMappingContext.h"
#include "InputData.h"
#include "SpawnActor.h"
#include "Blueprint/UserWidget.h"
#include "UI/EscapeMenu.h"
#include "Net/UnrealNetwork.h"
#include "Steam/steam_api.h"
#include "Steam/isteamnetworkingsockets.h"
#include "ER/FlecsSubsystem.h"
#include "Steam/isteamnetworkingutils.h"

AClientPlayerController::AClientPlayerController()
{
	// Find the InputMappingContext asset in the content browser
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextFinder(
		TEXT("/Game/Input/IMC_Default"));
	if (DefaultMappingContextFinder.Succeeded())
	{
		DefaultMappingContext = DefaultMappingContextFinder.Object;
		UE_LOG(LogTemp, Warning, TEXT("Found Mapping Context!"));
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MenuMappingContextFinder(
	TEXT("/Game/Input/IMC_Escape"));
	if (MenuMappingContextFinder.Succeeded())
	{
		MenuMappingContext = MenuMappingContextFinder.Object;
		UE_LOG(LogTemp, Warning, TEXT("Found Mapping Context!"));
	}

	// Find the InputActionDataAsset in the content browser
	static ConstructorHelpers::FObjectFinder<UInputData> InputDataAssetFinder(
		TEXT("/Game/Input/Data/DA_InputActions"));
	if (InputDataAssetFinder.Succeeded())
	{
		InputData = InputDataAssetFinder.Object;
		UE_LOG(LogTemp, Warning, TEXT("Found Input Actions!"));
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> EscapeMenuWidgetFinder(
		TEXT("/Game/Blueprints/UI/EscapeMenu"));
	if (EscapeMenuWidgetFinder.Succeeded())
	{
		EscapeMenuWidget = EscapeMenuWidgetFinder.Class;
		UE_LOG(LogTemp, Warning, TEXT("Found Escape Menu Widget!"));
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> LoadingScreenWidgetFinder(
	TEXT("/Game/Blueprints/UI/LoadingScreen"));
	if (LoadingScreenWidgetFinder.Succeeded())
	{
		LoadingScreenWidget = LoadingScreenWidgetFinder.Class;
		UE_LOG(LogTemp, Warning, TEXT("Found Loading Screen Widget!"));
	}
	
	bReplicates = true;
}

void AClientPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// // Only attempt to set up input on clients
	// if (!HasAuthority())
	// {
	// 	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	// 	{
	// 		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	// 		{
	// 			// Add the default mapping context
	// 			Subsystem->AddMappingContext(DefaultMappingContext, 0);
	// 			UE_LOG(LogTemp, Log, TEXT("Default mapping context added."));
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("Failed to get EnhancedInputLocalPlayerSubsystem."));
	// 		}
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("Unable to get LocalPlayer - check network setup."));
	// 	}
	// }
}

void AClientPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AClientPlayerController::ConnectToServer(const FString& ServerSteamID)
{
	if (!IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot connect from non-local player controller."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is null, cannot connect to server."));
		return;
	}

	// Validate the Steam ID
	if (ServerSteamID.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid Steam ID provided."));
		return;
	}

	if (LoadingScreenWidget)
	{
		if (!LoadingScreen)
		{
			LoadingScreen = CreateWidget<UUserWidget>(this, LoadingScreenWidget);
		}

		if (LoadingScreen)
		{
			LoadingScreen->AddToViewport();
		}
	}
	// Construct the Steam connection URL using the Steam ID
	FString TravelURL = FString::Printf(TEXT("steam.%s"), *ServerSteamID);

	// Log the connection attempt
	//UE_LOG(LogTemp, Log, TEXT("Attempting to connect to Steam server with SteamID: %s"), *ServerSteamID);

	// Use ClientTravel to connect to the server
	ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Connecting to server at %s:%d"), *ServerSteamID));
}

void AClientPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	//
	// // Ensure InputComponent is valid
	// if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	// {
	// 	if (InputData) {
	// 		if (InputData->ShowEscapeMenu)
	// 		{
	// 			EnhancedInputComponent->BindAction(InputData->ShowEscapeMenu, ETriggerEvent::Completed, this, &AClientPlayerController::OnShowEscapeMenu);
	// 			UE_LOG(LogTemp, Log, TEXT("Escape Menu action bound."));
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("ShowEscapeMenu is null."));
	// 		}
	//
	// 		if (InputData->LeftMouseClick)
	// 		{
	// 			EnhancedInputComponent->BindAction(InputData->LeftMouseClick, ETriggerEvent::Completed, this, &AClientPlayerController::LeftMouseClick);
	// 			UE_LOG(LogTemp, Log, TEXT("Left Mouse Click action bound."));
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("LeftMouseClick is null."));
	// 		}
	//
	// 		if (InputData->RightMouseClick)
	// 		{
	// 			EnhancedInputComponent->BindAction(InputData->RightMouseClick, ETriggerEvent::Completed, this, &AClientPlayerController::RightMouseClick);
	// 			UE_LOG(LogTemp, Log, TEXT("Right Mouse Click action bound."));
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("RightMouseClick is null."));
	// 		}
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("InputData is null."));
	// 	}
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found."));
	// }
}

void AClientPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

				// Re-enable all input mappings
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}

				SetInputMode(FInputModeGameOnly());
			}
			else
			{
				EscapeMenu->AddToViewport();
				UE_LOG(LogTemp, Log, TEXT("Escape Menu Widget displayed."));

				// Disable movement-related input mappings
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
				{
					// Remove all mapping contexts except the one for menu/escape
					Subsystem->RemoveMappingContext(DefaultMappingContext);
                 
					// If you have a specific menu mapping context, add it here
					if (MenuMappingContext)
					{
						Subsystem->AddMappingContext(MenuMappingContext, 0);
					}
				}

				// Custom input mode that keeps escape functional
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(EscapeMenu->TakeWidget());
				InputMode.SetHideCursorDuringCapture(false);
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

				SetInputMode(InputMode);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Escape Menu Widget is null."));
		}
	}
}

void AClientPlayerController::SpawnActors()
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

				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnActor::StaticClass(), FoundActors);

				for (AActor* Actor : FoundActors)
				{
					ASpawnActor* CustomActor = Cast<ASpawnActor>(Actor);
					if (CustomActor && CustomActor->ShowSpawnMenu(false))
					{
						// Material was reverted successfully
						UE_LOG(LogTemp, Warning, TEXT("Material reverted on %s"), *CustomActor->GetName());
					}
				}
				
				if (Spawner)
				{
					if (!Spawner->ShowSpawnMenu(true))
					{
						// Already using new material, so toggle back to the original
						Spawner->ShowSpawnMenu(false);
					}
				}
			}
			else
			{
				// No valid custom actor was hit, toggle all actors back to their original material
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnActor::StaticClass(), FoundActors);

				for (AActor* Actor : FoundActors)
				{
					ASpawnActor* CustomActor = Cast<ASpawnActor>(Actor);
					if (CustomActor && CustomActor->ShowSpawnMenu(false))
					{
						// Material was reverted successfully
						UE_LOG(LogTemp, Warning, TEXT("Material reverted on %s"), *CustomActor->GetName());
					}
				}
			}

			// Just a useful visual that is needed for now
			//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f, 0, 1.0f);
		}
	}
}


void AClientPlayerController::ServerRequestSpawnHorde_Implementation(FVector HordeSpawnLocation, float Radius, int32 NumEntities)
{
	// Ensure this runs on the server
	if (HasAuthority())
	{
		UFlecsSubsystem* FlecsSubsystem = GetGameInstance()->GetSubsystem<UFlecsSubsystem>();
		if (FlecsSubsystem)
		{
			FlecsSubsystem->SpawnZombieHorde(HordeSpawnLocation, Radius, NumEntities);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FlecsSubsystem not found on server!"));
		}
	}
}

bool AClientPlayerController::ServerRequestSpawnHorde_Validate(FVector HordeSpawnLocation, float Radius, int32 NumEntities)
{
	// Validate the request parameters (optional)
	return Radius > 0 && NumEntities > 0;
}