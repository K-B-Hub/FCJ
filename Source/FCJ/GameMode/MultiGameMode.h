// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FCJ_API AMultiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMultiGameMode();

protected:
	// Blueprint configurable player controller class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<class APlayerController> MultiPlayerControllerClass;

	// Blueprint configurable default pawn class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<class APawn> DefaultPawnClass_Multi;

public:
	// Override to set custom classes
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
}; 
