// Copyright Epic Games, Inc. All Rights Reserved.


#include "LocalGameMode.h"

#include "FCJ/PlayerCharacter/LocalPlayerCharacter.h"
#include "FCJ/PlayerController/LocalPlayerController.h"

ALocalGameMode::ALocalGameMode()
{
	PlayerControllerClass = DefaultPlayerControllerClass;
	DefaultPawnClass = DefaultPlayerPawnClass;
}
