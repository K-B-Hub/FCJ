// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SettingsWidget.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LocalPlayButton)
	{
		LocalPlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLocalPlayClicked);
	}

	if (MultiPlayButton)
	{
		MultiPlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnMultiPlayClicked);
	}

	if (SettingButton)
	{
		SettingButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);
	}

	if (SettingsWidget)
	{
		SettingsWidget->OnBackButtonClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsBackClicked);
		SettingsWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainMenuWidget::OnLocalPlayClicked()
{
    
}

void UMainMenuWidget::OnMultiPlayClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Levels/Test"), true);
}

void UMainMenuWidget::OnSettingClicked()
{
	if (SettingsWidget)
	{
		SettingsWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMainMenuWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, true);
}

void UMainMenuWidget::OnSettingsBackClicked()
{
	if (SettingsWidget)
	{
		SettingsWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
