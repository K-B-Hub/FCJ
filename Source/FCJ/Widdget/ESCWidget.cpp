// Fill out your copyright notice in the Description page of Project Settings.

#include "Widdget/ESCWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UESCWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize binding flag
	bButtonsAreBound = false;

	// Only bind if not already bound
	if (!bButtonsAreBound)
	{
		if (ResumeButton)
		{
			ResumeButton->OnClicked.AddDynamic(this, &UESCWidget::OnResumeClicked);
			UE_LOG(LogTemp, Warning, TEXT("ESCWidget: Bound ResumeButton"));
		}

		if (MainMenuButton)
		{
			MainMenuButton->OnClicked.AddDynamic(this, &UESCWidget::OnMainMenuClicked);
			UE_LOG(LogTemp, Warning, TEXT("ESCWidget: Bound MainMenuButton"));
		}

		if (ExitGameButton)
		{
			ExitGameButton->OnClicked.AddDynamic(this, &UESCWidget::OnExitGameClicked);
			UE_LOG(LogTemp, Warning, TEXT("ESCWidget: Bound ExitGameButton"));
		}

		bButtonsAreBound = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ESCWidget: Buttons already bound, skipping"));
	}
}

void UESCWidget::OnResumeClicked()
{
	OnResumeButtonClicked.Broadcast();
	SetVisibility(ESlateVisibility::Hidden);
}

void UESCWidget::OnMainMenuClicked()
{
	OnMainMenuButtonClicked.Broadcast();
}

void UESCWidget::OnExitGameClicked()
{
	OnExitGameButtonClicked.Broadcast();
}

void UESCWidget::NativeDestruct()
{
	UE_LOG(LogTemp, Warning, TEXT("ESCWidget: NativeDestruct called"));
	
	// Clear button bindings to prevent issues
	if (ResumeButton)
	{
		ResumeButton->OnClicked.RemoveAll(this);
	}
	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.RemoveAll(this);
	}
	if (ExitGameButton)
	{
		ExitGameButton->OnClicked.RemoveAll(this);
	}
	
	bButtonsAreBound = false;
	Super::NativeDestruct();
}
