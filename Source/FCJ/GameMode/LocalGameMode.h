// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LocalGameMode.generated.h"

class ALocalPlayerCharacter;
class ALocalPlayerController;
/**
 * 
 */
UCLASS()
class FCJ_API ALocalGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALocalGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	TSubclassOf<APlayerController> DefaultPlayerControllerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	TSubclassOf<ALocalPlayerCharacter> DefaultPlayerPawnClass;
	
};
