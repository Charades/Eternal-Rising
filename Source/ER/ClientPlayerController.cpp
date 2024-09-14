// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputData.h"
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

void AClientPlayerController::PollConnection()
{
	/*if (ServerConnection == k_HSteamNetConnection_Invalid)
	{
		return;
	}

	ISteamNetworkingMessage* IncomingMsg = nullptr;
	int NumMessages = SteamNetworkingSockets()->ReceiveMessagesOnConnection(ServerConnection, &IncomingMsg, 1);

	if (NumMessages > 0 && IncomingMsg)
	{
		FString ReceivedData = ANSI_TO_TCHAR((const char*)IncomingMsg->m_pData);
		UE_LOG(LogTemp, Log, TEXT("Received Message from Server: %s"), *ReceivedData);

		// You can process the received data here

		IncomingMsg->Release();
	}*/
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
				UE_LOG(LogTemp, Warning, TEXT("ShowServerBrowser is null."));
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

	//PollConnection();
	
	// Handles Steam callbacks on the client
	SteamAPI_RunCallbacks();
}

void AClientPlayerController::OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_Connected:
		UE_LOG(LogTemp, Log, TEXT("Connected to Server"));
		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		UE_LOG(LogTemp, Log, TEXT("Connection to Server Closed"));
		ServerConnection = k_HSteamNetConnection_Invalid;
		break;

	default:
		break;
	}
}

void AClientPlayerController::SendMessageToServer(const FString& Message)
{
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
