// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"
#include "FCJ/PlayerController/MainMenuController.h"
#include "FCJ/Widdget/MainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameUserSettings.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	PlayerControllerClass = AMainMenuController::StaticClass();
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Apply saved display settings on game startup
	ApplySavedDisplaySettings();

	if (MainMenuWidgetClass)
	{
		MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
		}
	}
}

void AMainMenuGameMode::ApplySavedDisplaySettings()
{
	UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (GameUserSettings)
	{
		GameUserSettings->LoadSettings();
		GameUserSettings->ApplySettings(false);
	}
}
