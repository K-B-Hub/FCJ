// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiSessionWidget.generated.h"

class UButton;
class UEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackButtonClicked);

/**
 *
 */
UCLASS()
class FCJ_API UMultiSessionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateServerButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinServerButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* SessionIdTextBox;

public:
	UPROPERTY(BlueprintAssignable)
	FOnBackButtonClicked OnBackButtonClicked;

private:
	UFUNCTION()
	void OnCreateServerClicked();

	UFUNCTION()
	void OnJoinServerClicked();

	UFUNCTION()
	void OnBackClicked();
};
