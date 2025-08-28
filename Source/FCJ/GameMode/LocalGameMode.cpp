// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalGameMode.h"
#include "PlayerController/LocalPlayerController.h"

ALocalGameMode::ALocalGameMode()
{
    PlayerControllerClass = ALocalPlayerController::StaticClass();
}
