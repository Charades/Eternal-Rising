#pragma once

#include "steam/steam_api.h"

class UClientNetworkSubsystem;

class FSteamServerCallback : public ISteamMatchmakingServerListResponse
{
public:
	FSteamServerCallback(UClientNetworkSubsystem* InSubsystem) : Subsystem(InSubsystem) {}

	// Called when a server responds to a query
	virtual void ServerResponded(HServerListRequest hRequest, int iServer) override;

	// Called when a server failed to respond to a query
	virtual void ServerFailedToRespond(HServerListRequest hRequest, int iServer) override;

	// Called when the server list is complete
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response) override;

private:
	UClientNetworkSubsystem* Subsystem;
};