// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerBrowser.h"
#include <string>
#include "ServerEntry.h"
#include "ClientNetworkSubsystem.h"
#include "ER/ClientPlayerController.h"
#include "Networking.h"

void UServerBrowser::NativeConstruct()
{
	Super::NativeConstruct();

	ServerList = Cast<UServerList>(GetWidgetFromName(TEXT("ServerList")));
	ConnectButton = Cast<UButton>(GetWidgetFromName(TEXT("ConnectButton")));
	RefreshButton = Cast<UButton>(GetWidgetFromName(TEXT("RefreshButton")));
	FullBool = Cast<UCheckBox>(GetWidgetFromName(TEXT("FullBool")));
	PlayersBool = Cast<UCheckBox>(GetWidgetFromName(TEXT("PlayersBool")));
	PasswordBool = Cast<UCheckBox>(GetWidgetFromName(TEXT("PasswordBool")));
	ServerTextBox = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("ServerTextBox")));
	MapTextBox = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("MapTextBox")));
	LatencyBox = Cast<UComboBoxString>(GetWidgetFromName(TEXT("LatencyBox")));
	InternetButton = Cast<UButton>(GetWidgetFromName(TEXT("InternetButton")));
	FavoritesButton = Cast<UButton>(GetWidgetFromName(TEXT("FavoritesButton")));
	AddFavoritesButton = Cast<UButton>(GetWidgetFromName(TEXT("AddFavoritesButton")));
	RemoveFavoritesButton = Cast<UButton>(GetWidgetFromName(TEXT("RemoveFavoritesButton")));
	ServerNameSort = Cast<UButton>(GetWidgetFromName(TEXT("ServerNameSort")));
	MapNameSort = Cast<UButton>(GetWidgetFromName(TEXT("MapNameSort")));
	PlayersSort = Cast<UButton>(GetWidgetFromName(TEXT("PlayersSort")));
	LatencySort = Cast<UButton>(GetWidgetFromName(TEXT("LatencySort")));
	
	if (ConnectButton)
	{
		ConnectButton->OnClicked.AddDynamic(this, &UServerBrowser::OnConnectButtonClicked);
	}
	
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UServerBrowser::OnRefreshButtonClicked);
	}

	if (ServerList)
	{
		ServerList->OnServerListItemSelectionChanged.AddUObject(this, &UServerBrowser::OnServerEntryClicked);
	}

	if (ServerTextBox)
	{
		ServerTextBox->OnTextChanged.AddDynamic(this, &UServerBrowser::OnServerTextChanged);
	}

	if (MapTextBox)
	{
		MapTextBox->OnTextChanged.AddDynamic(this, &UServerBrowser::OnMapTextChanged);
	}
	
	if (LatencyBox)
	{
		LatencyBox->OnSelectionChanged.AddDynamic(this, &UServerBrowser::OnLatencySelected);
	}

	if (InternetButton)
	{
		InternetButton->OnClicked.AddDynamic(this, &UServerBrowser::OnInternetButtonClicked);
		SetButtonActive(InternetButton);
	}

	if (FavoritesButton)
	{
		FavoritesButton->OnClicked.AddDynamic(this, &UServerBrowser::OnFavoritesButtonClicked);
	}	

	if (AddFavoritesButton)
	{
		AddFavoritesButton->SetVisibility(ESlateVisibility::Visible);
		AddFavoritesButton->OnClicked.AddDynamic(this, &UServerBrowser::OnAddFavoritesButtonClicked);
	}

	if (RemoveFavoritesButton)
	{
		RemoveFavoritesButton->SetVisibility(ESlateVisibility::Collapsed);
		RemoveFavoritesButton->OnClicked.AddDynamic(this, &UServerBrowser::OnRemoveFavoritesButtonClicked);
	}

	if (ServerNameSort)
	{
		ServerNameSort->OnClicked.AddDynamic(this, &UServerBrowser::OnServerNameSortClicked);
	}

	if (MapNameSort)
	{
		MapNameSort->OnClicked.AddDynamic(this, &UServerBrowser::OnMapNameSortClicked);
	}

	if (PlayersSort)
	{
		PlayersSort->OnClicked.AddDynamic(this, &UServerBrowser::OnPlayersSortClicked);
	}

	if (LatencySort)
	{
		LatencySort->OnClicked.AddDynamic(this, &UServerBrowser::OnLatencySortClicked);
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			ClientNetworkSubsystem->OnServerListUpdated.AddDynamic(this, &UServerBrowser::UpdateServerList);
		}
	}
}

void UServerBrowser::OnServerEntryClicked(USteamServerWrapper* Server, bool isSelected)
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Server entry selected"));
	
	if (Server)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Server->GetSteamID());
		SelectedServer = Server;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Connection Info is valid"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Connection Info is invalid"));
	}
}

void UServerBrowser::OnConnectButtonClicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Connect Button Clicked"));
	
	if (SelectedServer->GetSteamID().IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("No server selected."));
		return;
	}

	// Get the PlayerController and cast it to AMyPlayerController
	AClientPlayerController* MyPC = Cast<AClientPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyPC)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("PC ConnectToServer called."));
		MyPC->ConnectToServer(SelectedServer->GetSteamID());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Failed to get custom PlayerController."));
	}
}

void UServerBrowser::OnRefreshButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Refresh button clicked!"));
	}

	RefreshServerList();
}

void UServerBrowser::PopulateServerList(const TArray<USteamServerWrapper*>& Servers)
{
	if (!ServerList) return;

	SelectedServer = nullptr;
	ServerList->ClearListItems();
	ServerList->SetListItems(Servers);
}

void UServerBrowser::UpdateServerList()
{
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		ServerList->ClearSelections();
		
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
			if (ClientNetworkSubsystem)
			{
				PopulateServerList(ClientNetworkSubsystem->GameServerList);
			}
		}
	});
}

void UServerBrowser::RefreshServerList()
{
	ServerList->ClearListItems();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			TArray<FString> Keys = { "gamedir" };
			TArray<FString> Values = { "spacewar" };

			if (FullBool->IsChecked())
			{
				Keys.Add("notfull");
				Values.Add("1");
			}

			if (PlayersBool->IsChecked())
			{
				Keys.Add("hasplayers");
				Values.Add(TEXT(""));
			}

			if (!bFavoriteSelected)
			{
				ClientNetworkSubsystem->RequestInternetServerList(Keys, Values, PasswordBool->IsChecked());
			}
			else
			{
				ClientNetworkSubsystem->RequestFavoriteServerList(Keys, Values, PasswordBool->IsChecked());
			}
			
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

void UServerBrowser::OnServerTextChanged(const FText& Text)
{
	FString FilterText = Text.ToString().ToLower(); // Get the filter text in lowercase

	// Filter the servers based on the name matching the filter text
	TArray<USteamServerWrapper*> FilteredServers;
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			for (USteamServerWrapper* Server : ClientNetworkSubsystem->GameServerList)  // Assuming 'GameServerList' is the list of servers
			{
				if (Server && Server->GetName().ToLower().Contains(FilterText))  // Case-insensitive match
				{
					FilteredServers.Add(Server);
				}
			}
		}
	}

	// Call PopulateServerList with the filtered servers
	PopulateServerList(FilteredServers);
}

void UServerBrowser::OnMapTextChanged(const FText& Text)
{
	FString FilterText = Text.ToString().ToLower(); // Get the filter text in lowercase

	// Filter the servers based on the name matching the filter text
	TArray<USteamServerWrapper*> FilteredServers;
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			for (USteamServerWrapper* Server : ClientNetworkSubsystem->GameServerList)  // Assuming 'GameServerList' is the list of servers
			{
				if (Server && Server->GetMap().ToLower().Contains(FilterText))  // Case-insensitive match
				{
					FilteredServers.Add(Server);
				}
			}
		}
	}

	// Call PopulateServerList with the filtered servers
	PopulateServerList(FilteredServers);
}

void UServerBrowser::OnLatencySelected(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// Parse the selected option to get the latency threshold
	int32 LatencyThreshold = 0;

	// Compare the selected option and set the appropriate threshold
	if (SelectedItem == TEXT("< 50"))
	{
		LatencyThreshold = 50;
	}
	else if (SelectedItem == TEXT("< 100"))
	{
		LatencyThreshold = 100;
	}
	else if (SelectedItem == TEXT("< 150"))
	{
		LatencyThreshold = 150;
	}
	else if (SelectedItem == TEXT("< 200"))
	{
		LatencyThreshold = 200;
	}
	else
	{
		// Handle case for "All" or no selection, if needed
		LatencyThreshold = -1;  // This would mean no filtering based on latency
	}
	
	FilterServersByLatency(LatencyThreshold);
}

void UServerBrowser::FilterServersByLatency(int32 LatencyThreshold)
{
	// Create an array to store the filtered servers
	TArray<USteamServerWrapper*> FilteredServers;

	// Loop through all the servers and apply the latency filter
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
		if (ClientNetworkSubsystem)
		{
			for (USteamServerWrapper* Server : ClientNetworkSubsystem->GameServerList)  // Assuming GameServerList contains all servers
			{
				if (Server)
				{
					int32 ServerPing = Server->GetPing();
					if (LatencyThreshold == -1 || ServerPing < LatencyThreshold)  // If no filtering, or server ping is under the threshold
					{
						FilteredServers.Add(Server);
					}
				}
			}
		}
	}
	
	PopulateServerList(FilteredServers);
}

void UServerBrowser::OnInternetButtonClicked()
{
	// When InternetButton is clicked, set it active and FavoritesButton inactive
	SetButtonActive(InternetButton);
	SetButtonInactive(FavoritesButton);
	bFavoriteSelected = false;
	RemoveFavoritesButton->SetVisibility(ESlateVisibility::Collapsed);
	AddFavoritesButton->SetVisibility(ESlateVisibility::Visible);
	RefreshServerList();
}

void UServerBrowser::OnFavoritesButtonClicked()
{
	// When FavoritesButton is clicked, set it active and InternetButton inactive
	SetButtonActive(FavoritesButton);
	SetButtonInactive(InternetButton);
	bFavoriteSelected = true;
	AddFavoritesButton->SetVisibility(ESlateVisibility::Collapsed);
	RemoveFavoritesButton->SetVisibility(ESlateVisibility::Visible);
	RefreshServerList();
}

void UServerBrowser::OnAddFavoritesButtonClicked()
{
	if (SelectedServer)
	{
		FString ServerIP = SelectedServer->GetIP(); // Replace with actual method to get server's IP
		int32 ConnectionPort = SelectedServer->GetPort(); // Replace with actual method to get server's port
		int32 QueryPort = SelectedServer->GetQueryPort(); // Replace with actual method to get server's port
        
		// Ensure we have a valid IP and port
		if (!ServerIP.IsEmpty() && ConnectionPort > 0)
		{
			// Convert FString to uint32 for Steam's expected IP format
			FIPv4Address Address;
			if (FIPv4Address::Parse(ServerIP, Address))
			{
				uint32 HostOrderIP = Address.Value; // Converts IP address to host order (uint32)

				// Get the Steam App ID for the game (replace with your App ID)
				AppId_t AppID = 480; // Example: App ID for SpaceWar; replace with your own game ID

				// Assuming the connection port is the same as the query port for simplicity
				uint16 nConnPort = static_cast<uint16>(ConnectionPort);
				uint16 nQueryPort = static_cast<uint16>(QueryPort);

				// Add the server to the favorites list
				SteamMatchmaking()->AddFavoriteGame(AppID, HostOrderIP, nConnPort, nQueryPort, k_unFavoriteFlagFavorite, FDateTime::UtcNow().ToUnixTimestamp());

				FString DebugMessage = FString::Printf(TEXT("Server added to favorites: %s:%d"), *ServerIP, ConnectionPort);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, DebugMessage);
			}
		}	
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Add Favorite: No server selected."));
	}
}

void UServerBrowser::OnRemoveFavoritesButtonClicked()
{
	if (SelectedServer)
	{
		FString ServerIP = SelectedServer->GetIP(); // Replace with actual method to get server's IP
		int32 ConnectionPort = SelectedServer->GetPort(); // Replace with actual method to get server's port
		int32 QueryPort = SelectedServer->GetQueryPort(); // Replace with actual method to get server's port
        
		// Ensure we have a valid IP and port
		if (!ServerIP.IsEmpty() && ConnectionPort > 0)
		{
			// Convert FString to uint32 for Steam's expected IP format
			FIPv4Address Address;
			if (FIPv4Address::Parse(ServerIP, Address))
			{
				uint32 HostOrderIP = Address.Value; // Converts IP address to host order (uint32)

				// Get the Steam App ID for the game (replace with your App ID)
				AppId_t AppID = 480; // Example: App ID for SpaceWar; replace with your own game ID

				// Assuming the connection port is the same as the query port for simplicity
				uint16 nConnPort = static_cast<uint16>(ConnectionPort);
				uint16 nQueryPort = static_cast<uint16>(QueryPort);

				// Add the server to the favorites list
				SteamMatchmaking()->RemoveFavoriteGame(AppID, HostOrderIP, nConnPort, nQueryPort, k_unFavoriteFlagFavorite);

				FString DebugMessage = FString::Printf(TEXT("Server removed from favorites: %s:%d"), *ServerIP, ConnectionPort);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, DebugMessage);
			}
		}	
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Add Favorite: No server selected."));
	}

	if (bFavoriteSelected)
	{
		RefreshServerList();
	}
}

void UServerBrowser::OnServerNameSortClicked()
{
	// Toggle the sort order (ascending or descending)
	bIsAscendingOrder = !bIsAscendingOrder;

	// Sort the server list based on server name
	SortServersByName(bIsAscendingOrder);
}

void UServerBrowser::SortServersByName(bool bAscending)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
	if (!ClientNetworkSubsystem) return;

	// Create a copy of the server list that we can sort
	TArray<USteamServerWrapper*> SortedServers = ClientNetworkSubsystem->GameServerList;

	// Sort the array based on server names
	SortedServers.Sort([bAscending](const USteamServerWrapper& A, const USteamServerWrapper& B) {
		// Get server names and convert to lowercase for case-insensitive comparison
		FString NameA = A.GetName().ToLower();
		FString NameB = B.GetName().ToLower();
        
		if (bAscending)
		{
			return NameA < NameB;
		}
		else
		{
			return NameA > NameB;
		}
	});

	// Update the UI with the sorted server list
	PopulateServerList(SortedServers);
}

void UServerBrowser::OnMapNameSortClicked()
{
	// Toggle the sort order (ascending or descending)
	bIsAscendingOrder = !bIsAscendingOrder;

	// Sort the server list based on map name
	SortServersByMap(bIsAscendingOrder);
}

void UServerBrowser::SortServersByMap(bool bAscending)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
	if (!ClientNetworkSubsystem) return;

	// Create a copy of the server list that we can sort
	TArray<USteamServerWrapper*> SortedServers = ClientNetworkSubsystem->GameServerList;

	// Sort the array based on map names
	SortedServers.Sort([bAscending](const USteamServerWrapper& A, const USteamServerWrapper& B) {
		// Get map names and convert to lowercase for case-insensitive comparison
		FString MapA = A.GetMap().ToLower();
		FString MapB = B.GetMap().ToLower();

		// If map names are the same, sort by server name
		if (MapA == MapB)
		{
			FString NameA = A.GetName().ToLower();
			FString NameB = B.GetName().ToLower();
			return NameA < NameB;
		}
		
		if (bAscending)
		{
			return MapA < MapB;
		}
		else
		{
			return MapA > MapB;
		}
	});

	// Update the UI with the sorted server list
	PopulateServerList(SortedServers);
}


void UServerBrowser::OnPlayersSortClicked()
{
	// Toggle the sort order (ascending or descending)
	bIsAscendingOrder = !bIsAscendingOrder;

	// Sort the server list based on player count
	SortServersByPlayers(bIsAscendingOrder);
}

void UServerBrowser::SortServersByPlayers(bool bAscending)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
	if (!ClientNetworkSubsystem) return;

	// Create a copy of the server list that we can sort
	TArray<USteamServerWrapper*> SortedServers = ClientNetworkSubsystem->GameServerList;

	// Sort the array based on player count
	SortedServers.Sort([bAscending](const USteamServerWrapper& A, const USteamServerWrapper& B) {
		// Get player counts from each server
		int32 PlayersA = A.GetPlayers();
		int32 PlayersB = B.GetPlayers();
        
		if (bAscending)
		{
			// If player counts are equal, sort by server name as a secondary criteria
			if (PlayersA == PlayersB)
			{
				return A.GetName().ToLower() < B.GetName().ToLower();
			}
			return PlayersA < PlayersB;
		}
		else
		{
			// If player counts are equal, sort by server name as a secondary criteria
			if (PlayersA == PlayersB)
			{
				return A.GetName().ToLower() < B.GetName().ToLower();
			}
			return PlayersA > PlayersB;
		}
	});

	// Update the UI with the sorted server list
	PopulateServerList(SortedServers);
}

void UServerBrowser::OnLatencySortClicked()
{
	// Toggle the sort order (ascending or descending)
	bIsAscendingOrder = !bIsAscendingOrder;

	// Sort the server list based on player count
	SortServersByLatency(bIsAscendingOrder);
}

void UServerBrowser::SortServersByLatency(bool bAscending)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UClientNetworkSubsystem* ClientNetworkSubsystem = GameInstance->GetSubsystem<UClientNetworkSubsystem>();
	if (!ClientNetworkSubsystem) return;

	// Create a copy of the server list that we can sort
	TArray<USteamServerWrapper*> SortedServers = ClientNetworkSubsystem->GameServerList;

	// Sort the array based on player count
	SortedServers.Sort([bAscending](const USteamServerWrapper& A, const USteamServerWrapper& B) {
		// Get player counts from each server
		int32 PlayersA = A.GetPing();
		int32 PlayersB = B.GetPing();
        
		if (bAscending)
		{
			// If latency is equal, sort by server name as a secondary criteria
			if (PlayersA == PlayersB)
			{
				return A.GetName().ToLower() < B.GetName().ToLower();
			}
			return PlayersA < PlayersB;
		}
		else
		{
			// If latency is equal, sort by server name as a secondary criteria
			if (PlayersA == PlayersB)
			{
				return A.GetName().ToLower() < B.GetName().ToLower();
			}
			return PlayersA > PlayersB;
		}
	});

	// Update the UI with the sorted server list
	PopulateServerList(SortedServers);
}

void UServerBrowser::SetButtonActive(UButton* Button)
{
	// Change the button's visual appearance (e.g., background color, border color, etc.)
	if (Button)
	{
		// Example: Change the button's appearance to indicate it's active
		Button->SetBackgroundColor(FLinearColor(0.670588f, 0.313726f, 0.0f, 1.0f));
	}
}

void UServerBrowser::SetButtonInactive(UButton* Button)
{
	// Reset the button's appearance when inactive
	if (Button)
	{
		Button->SetBackgroundColor(FLinearColor(0.08, 0.08, 0.08, 1.0f)); // Apply your custom style here
	}
}