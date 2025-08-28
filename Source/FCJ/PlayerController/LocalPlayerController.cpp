
#include "PlayerController/LocalPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "PlayerCharacter/LocalPlayerCharacter.h"
#include "PlayerCharacter/FrontCat.h"
#include "PlayerCharacter/BackCat.h"

ALocalPlayerController::ALocalPlayerController()
{
}

void ALocalPlayerController::OnPossess(APawn* aPawn)
{
    Super::OnPossess(aPawn);

    PlayerCharacter = Cast<ALocalPlayerCharacter>(aPawn);
    if (PlayerCharacter == nullptr)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
}

void ALocalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(MoveFrontAction, ETriggerEvent::Triggered, this, &ALocalPlayerController::MoveFront);
        EnhancedInputComponent->BindAction(JumpFrontAction, ETriggerEvent::Started, this, &ALocalPlayerController::JumpFront);
        EnhancedInputComponent->BindAction(MoveBackAction, ETriggerEvent::Triggered, this, &ALocalPlayerController::MoveBack);
        EnhancedInputComponent->BindAction(JumpBackAction, ETriggerEvent::Started, this, &ALocalPlayerController::JumpBack);
    }
}

void ALocalPlayerController::MoveFront(const FInputActionValue& Value)
{
    if (PlayerCharacter && PlayerCharacter->GetFrontCat())
        if (PlayerCharacter)
        {
            PlayerCharacter->HandleFrontCatMovement(Value);
        }
}

void ALocalPlayerController::JumpFront()
{
    if (PlayerCharacter && PlayerCharacter->GetFrontCat())
    {
        PlayerCharacter->GetFrontCat()->Jump();
    }
}

void ALocalPlayerController::MoveBack(const FInputActionValue& Value)
{
    if (PlayerCharacter)
    {
        // LocalPlayerCharacter에게 입력 전달
        PlayerCharacter->HandleBackCatMovement(Value);
    }
}

void ALocalPlayerController::JumpBack()
{
    if (PlayerCharacter && PlayerCharacter->GetBackCat())
    {
        PlayerCharacter->GetBackCat()->Jump();
    }
}
