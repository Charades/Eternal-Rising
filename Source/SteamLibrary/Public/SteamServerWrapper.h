#pragma once

#include "CoreMinimal.h"
#include "steam/steam_api.h"
#include "UObject/Object.h"
#include "SteamServerWrapper.generated.h"

UCLASS(BlueprintType)
class STEAMLIBRARY_API USteamServerWrapper : public UObject
{
    GENERATED_BODY()

public:
    USteamServerWrapper();
    
    void Initialize(gameserveritem_t* pGameServerItem);

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    FString IPAddress;  // IP address for the server in string format

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int32 ConnectionPort;  // Port for game clients to connect to for this server

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int32 QueryPort;  // Port for game clients to connect to for this server

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int32 Ping;  // current ping time in milliseconds

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    FString Map;  // current map

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    FString GameDescription;  // game description

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int32 Players;  // current number of players on the server

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int32 MaxPlayers;  // Maximum players that can join this server

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int32 BotPlayers;  // Number of bots (i.e., simulated players) on this server

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    bool bPassword;  // true if this server needs a password to join

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    bool bSecure;  // Is this server protected by VAC

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int32 ServerVersion;  // server version as reported to Steam

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    FString ServerName;  // Game server name

    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    FString ServerString;  // String to show in server browser
    
    UPROPERTY(BlueprintReadOnly, Category = "Game Server")
    int64 SteamID;  // Steam ID used for Steam IP shielding ie ConnectP2P

    UFUNCTION(BlueprintCallable, Category = "Game Server")
    FString GetName() const;

    UFUNCTION(BlueprintCallable, Category = "Game Server")
    FString GetMap() const;
    
    UFUNCTION(BlueprintCallable, Category = "Game Server")
    FString GetPlayerCount() const;
    
    UFUNCTION(BlueprintCallable, Category = "Game Server")
    FString GetDisplayString() const;

    UFUNCTION(BlueprintCallable, Category = "Game Server")
    FString GetIP() const;

    UFUNCTION(BlueprintCallable, Category = "Game Server")
    int32 GetPort() const;

    UFUNCTION(BlueprintCallable, Category = "Game Server")
    int32 GetQueryPort() const;

    UFUNCTION(BlueprintCallable, Category = "Game Server")
    int32 GetPlayers() const;
    
    UFUNCTION(BlueprintCallable, Category = "Game Server")
    int32 GetPing() const;

    UFUNCTION(BlueprintCallable, Category = "Game Server")
    FString GetSteamID() const;
    
    UFUNCTION(BlueprintCallable, Category = "Game Server")
    FString ConnectionIP() const;

private:
    FString ConvertIPToString(uint32 IP) const;
};