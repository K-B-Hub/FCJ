// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Components/Button.h"

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
}

void UMainMenuWidget::OnLocalPlayClicked()
{
}

void UMainMenuWidget::OnMultiPlayClicked()
{
}

void UMainMenuWidget::OnSettingClicked()
{
}

void UMainMenuWidget::OnExitClicked()
{
}
