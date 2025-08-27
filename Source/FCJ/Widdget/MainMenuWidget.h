// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class FCJ_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* LocalPlayButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MultiPlayButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SettingButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

private:
	UFUNCTION()
	void OnLocalPlayClicked();

	UFUNCTION()
	void OnMultiPlayClicked();

	UFUNCTION()
	void OnSettingClicked();

	UFUNCTION()
	void OnExitClicked();
	
};
