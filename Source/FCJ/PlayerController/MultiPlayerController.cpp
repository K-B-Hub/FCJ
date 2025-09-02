// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MultiPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter/CatBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

AMultiPlayerController::AMultiPlayerController()
{
	// Set default values
	MouseSensitivity = 1.0f;
	bInvertMouseY = false;
	ZoomSpeed = 50.0f;
	MinZoomDistance = 100.0f;
	MaxZoomDistance = 800.0f;
}

void AMultiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMultiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Moving
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiPlayerController::Move);
		}

		// Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMultiPlayerController::Look);
		}

		// Jumping
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMultiPlayerController::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMultiPlayerController::StopJumping);
		}

		// Special Action
		if (SpecialAction)
		{
			EnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Started, this, &AMultiPlayerController::PerformSpecialAction);
		}

		// Zoom
		if (ZoomAction)
		{
			EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AMultiPlayerController::Zoom);
		}
	}
}

void AMultiPlayerController::Move(const FInputActionValue& Value)
{
	// Input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (APawn* ControlledPawn = GetPawn())
	{
		if (ACharacter* character = Cast<ACharacter>(ControlledPawn))
		{
			// Use camera/controller rotation for movement direction
			const FRotator Rotation = GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// Get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			
			// Get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// Add movement 
			character->AddMovementInput(ForwardDirection, MovementVector.Y);
			character->AddMovementInput(RightDirection, MovementVector.X);
		}
	}
}

void AMultiPlayerController::Look(const FInputActionValue& Value)
{
	// Input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Apply sensitivity
	LookAxisVector *= MouseSensitivity;

	// Invert Y axis if needed
	if (bInvertMouseY)
	{
		LookAxisVector.Y *= -1.0f;
	}

	// Add yaw and pitch input to controller (no limits)
	AddYawInput(LookAxisVector.X);
	AddPitchInput(-LookAxisVector.Y);
}

void AMultiPlayerController::Jump()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (ACharacter* character = Cast<ACharacter>(ControlledPawn))
		{
			character->Jump();
		}
	}
}

void AMultiPlayerController::StopJumping()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (ACharacter* character = Cast<ACharacter>(ControlledPawn))
		{
			character->StopJumping();
		}
	}
}

void AMultiPlayerController::PerformSpecialAction()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (ACatBase* CatCharacter = Cast<ACatBase>(ControlledPawn))
		{
			CatCharacter->PerformSpecialAction();
		}
	}
}

void AMultiPlayerController::Zoom(const FInputActionValue& Value)
{
	// Input is a float (mouse wheel delta)
	float ZoomValue = Value.Get<float>();

	if (APawn* ControlledPawn = GetPawn())
	{
		if (ACatBase* CatCharacter = Cast<ACatBase>(ControlledPawn))
		{
			if (USpringArmComponent* SpringArm = CatCharacter->GetSpringArmComponent())
			{
				// Calculate new target arm length
				float CurrentLength = SpringArm->TargetArmLength;
				float NewLength = CurrentLength - (ZoomValue * ZoomSpeed);
				
				// Clamp to min/max values
				NewLength = FMath::Clamp(NewLength, MinZoomDistance, MaxZoomDistance);
				
				// Apply the new length
				SpringArm->TargetArmLength = NewLength;
			}
		}
	}
}

