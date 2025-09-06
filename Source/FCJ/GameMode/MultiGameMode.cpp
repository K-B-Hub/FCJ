// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MultiGameMode.h"
#include "PlayerController/MultiPlayerController.h"
#include "PlayerCharacter/CatBase.h"
#include "GameFramework/GameUserSettings.h"

AMultiGameMode::AMultiGameMode()
{
	// Set default classes (can be overridden in Blueprint)
	PlayerControllerClass = AMultiPlayerController::StaticClass();
	DefaultPawnClass = ACatBase::StaticClass();
}

void AMultiGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Apply Blueprint-configured classes if they are set
	if (MultiPlayerControllerClass)
	{
		PlayerControllerClass = MultiPlayerControllerClass;
	}

	if (DefaultPawnClass_Multi)
	{
		DefaultPawnClass = DefaultPawnClass_Multi;
	}
}

void AMultiGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Apply saved display settings on game startup
	ApplySavedDisplaySettings();
}

void AMultiGameMode::ApplySavedDisplaySettings()
{
	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (GameUserSettings)
	{
		GameUserSettings->LoadSettings();
		GameUserSettings->ApplySettings(false);
	}
}

