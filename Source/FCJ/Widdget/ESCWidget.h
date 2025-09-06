// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ESCWidget.generated.h"

class UButton;

UCLASS()
class FCJ_API UESCWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitGameButton;

private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnExitGameClicked();

	bool bButtonsAreBound;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResumeClicked);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMainMenuClicked);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitGameClicked);

	UPROPERTY(BlueprintAssignable)
	FOnResumeClicked OnResumeButtonClicked;

	UPROPERTY(BlueprintAssignable)
	FOnMainMenuClicked OnMainMenuButtonClicked;

	UPROPERTY(BlueprintAssignable)
	FOnExitGameClicked OnExitGameButtonClicked;
};
