// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ServerBrowser.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "ImageUtils.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemSteam.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MainMenu.generated.h"

UCLASS()
class ER_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UButton* ServerBrowserButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitGameButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* SettingsMenuButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SteamProfileButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MusicToggleButton;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AccountNameText;

	UPROPERTY(meta = (BindWidget))
	UImage* AvatarImage;

	UPROPERTY(meta = (BindWidget))
	UImage* AudioImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
	UTexture2D* MutedAudioImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Images")
	UTexture2D* UnmutedAudioImage;
	
	UMainMenu(const FObjectInitializer& ObjectInitializer);
	
	void UpdateSteamInfo();
	void GetSteamAvatar();
	void RemoveFromParent();
	
	UFUNCTION()
	void OnServerBrowserButtonClicked();

	UFUNCTION()
	void OnSettingsMenuButtonClicked();

	UFUNCTION()
	void OnExitGameButtonClicked();

	UFUNCTION()
	void OnSteamProfileButtonClicked();

protected:
	void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundCue* BackgroundMusic;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StartBackgroundMusic(USoundCue* MusicToPlay);
    
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopBackgroundMusic();
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UServerBrowser> ServerBrowserWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> SettingsMenuWidget;
	
	UPROPERTY()
	UServerBrowser* ServerBrowser;

	UPROPERTY()
	UUserWidget* SettingsMenu;

	UPROPERTY()
	UAudioComponent* AudioComponent;

	bool PlayingMusic = true;
};
