// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 *
 */
UCLASS()
class FCJ_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

	// 플레이어 역할 교체
	UFUNCTION(BlueprintCallable)
	void SwapPlayerRoles();

	// GameState 업데이트
	void UpdateGameStateRoles();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	// Apply saved display settings
	void ApplySavedDisplaySettings();
};
