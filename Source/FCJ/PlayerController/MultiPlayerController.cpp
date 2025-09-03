// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/MultiPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter/CatBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "InputMappingContext.h"
#include "Widdget/SettingsWidget.h"
#include "InputModifiers.h"

// Define static member
const FString AMultiPlayerController::InputSettingsSection = TEXT("FCJ.InputSettings");

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

	// Load input settings from config
	LoadInputSettings();

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
		// Individual Movement Actions
		if (MoveForwardAction)
		{
			EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AMultiPlayerController::MoveForward);
			EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Completed, this, &AMultiPlayerController::MoveForward);
		}
		
		if (MoveBackwardAction)
		{
			EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &AMultiPlayerController::MoveBackward);
			EnhancedInputComponent->BindAction(MoveBackwardAction, ETriggerEvent::Completed, this, &AMultiPlayerController::MoveBackward);
		}
		
		if (MoveLeftAction)
		{
			EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &AMultiPlayerController::MoveLeft);
			EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Completed, this, &AMultiPlayerController::MoveLeft);
		}
		
		if (MoveRightAction)
		{
			EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AMultiPlayerController::MoveRight);
			EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Completed, this, &AMultiPlayerController::MoveRight);
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


// Individual movement functions
void AMultiPlayerController::MoveForward(const FInputActionValue& Value)
{
	ForwardInputValue = Value.Get<float>();
	ApplyCombinedMovement();
	UE_LOG(LogTemp, Warning, TEXT("MoveForward: %f"), ForwardInputValue);
}

void AMultiPlayerController::MoveBackward(const FInputActionValue& Value)
{
	BackwardInputValue = Value.Get<float>();
	ApplyCombinedMovement();
	UE_LOG(LogTemp, Warning, TEXT("MoveBackward: %f"), BackwardInputValue);
}

void AMultiPlayerController::MoveLeft(const FInputActionValue& Value)
{
	LeftInputValue = Value.Get<float>();
	ApplyCombinedMovement();
	UE_LOG(LogTemp, Warning, TEXT("MoveLeft: %f"), LeftInputValue);
}

void AMultiPlayerController::MoveRight(const FInputActionValue& Value)
{
	RightInputValue = Value.Get<float>();
	ApplyCombinedMovement();
	UE_LOG(LogTemp, Warning, TEXT("MoveRight: %f"), RightInputValue);
}

void AMultiPlayerController::ApplyCombinedMovement()
{
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

			// Calculate net movement values
			float NetForwardInput = ForwardInputValue - BackwardInputValue;
			float NetRightInput = RightInputValue - LeftInputValue;

			// Apply combined movement
			character->AddMovementInput(ForwardDirection, NetForwardInput);
			character->AddMovementInput(RightDirection, NetRightInput);
			
			if (NetForwardInput != 0.0f || NetRightInput != 0.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("Combined movement - Forward: %f, Right: %f"), NetForwardInput, NetRightInput);
			}
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

void AMultiPlayerController::LoadInputSettings()
{
	// Use SettingsWidget's static function to load settings
	TMap<FString, FString> KeyMappings;
	USettingsWidget::LoadInputSettingsFromConfig(MouseSensitivity, bInvertMouseY, ZoomSpeed, KeyMappings);
	
	// Apply key mappings to the input system
	ApplyKeyMappingsFromConfig(KeyMappings);
}

void AMultiPlayerController::SaveInputSettings()
{
	// This function now primarily for compatibility
	// Actual saving is handled by SettingsWidget's static functions
	TMap<FString, FString> CurrentKeyMappings;
	// TODO: Get current key mappings if needed
	USettingsWidget::SaveInputSettingsToConfig(MouseSensitivity, bInvertMouseY, ZoomSpeed, CurrentKeyMappings);
}

void AMultiPlayerController::ApplyKeyRemapping()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			// Remove and re-add the mapping context to apply changes
			Subsystem->RemoveMappingContext(DefaultMappingContext);
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

TArray<FKey> AMultiPlayerController::GetDefaultKeysForAction(UInputAction* Action)
{
	TArray<FKey> Keys;
	
	if (DefaultMappingContext && Action)
	{
		const TArray<FEnhancedActionKeyMapping>& Mappings = DefaultMappingContext->GetMappings();
		for (const FEnhancedActionKeyMapping& Mapping : Mappings)
		{
			if (Mapping.Action == Action)
			{
				Keys.Add(Mapping.Key);
			}
		}
	}
	
	return Keys;
}

void AMultiPlayerController::SetKeyForAction(UInputAction* Action, const FKey& NewKey)
{
	if (DefaultMappingContext && Action)
	{
		// Remove existing mappings for this action
		TArray<FEnhancedActionKeyMapping> Mappings = DefaultMappingContext->GetMappings();
		for (int32 i = Mappings.Num() - 1; i >= 0; i--)
		{
			if (Mappings[i].Action == Action)
			{
				DefaultMappingContext->UnmapKey(Action, Mappings[i].Key);
			}
		}
		
		// Add new mapping
		FEnhancedActionKeyMapping NewMapping;
		NewMapping.Action = Action;
		NewMapping.Key = NewKey;
		DefaultMappingContext->MapKey(Action, NewKey);
		
		// Apply the changes
		ApplyKeyRemapping();
		
		// Save the key mapping to config
		if (GConfig)
		{
			FString ActionName = Action->GetName();
			GConfig->SetString(*InputSettingsSection, *ActionName, *NewKey.ToString(), GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
	}
}

void AMultiPlayerController::ApplyKeyMappingsFromConfig(const TMap<FString, FString>& KeyMappings)
{
	if (!DefaultMappingContext)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Applying key mappings from config. Total mappings: %d"), KeyMappings.Num());
	
	// Apply key mappings to input actions
	for (const auto& Pair : KeyMappings)
	{
		FKey NewKey(*Pair.Value);
		UE_LOG(LogTemp, Warning, TEXT("Processing key mapping: %s -> %s"), *Pair.Key, *Pair.Value);
		
		if (NewKey.IsValid())
		{
			// Individual Movement Actions
			if (Pair.Key == TEXT("MoveForward") && MoveForwardAction)
			{
				UE_LOG(LogTemp, Warning, TEXT("Applying MoveForward key: %s"), *NewKey.ToString());
				SetKeyForAction(MoveForwardAction, NewKey);
			}
			else if (Pair.Key == TEXT("MoveBackward") && MoveBackwardAction)
			{
				UE_LOG(LogTemp, Warning, TEXT("Applying MoveBackward key: %s"), *NewKey.ToString());
				SetKeyForAction(MoveBackwardAction, NewKey);
			}
			else if (Pair.Key == TEXT("MoveLeft") && MoveLeftAction)
			{
				UE_LOG(LogTemp, Warning, TEXT("Applying MoveLeft key: %s"), *NewKey.ToString());
				SetKeyForAction(MoveLeftAction, NewKey);
			}
			else if (Pair.Key == TEXT("MoveRight") && MoveRightAction)
			{
				UE_LOG(LogTemp, Warning, TEXT("Applying MoveRight key: %s"), *NewKey.ToString());
				SetKeyForAction(MoveRightAction, NewKey);
			}
			else if (Pair.Key == TEXT("Jump") && JumpAction)
			{
				UE_LOG(LogTemp, Warning, TEXT("Applying Jump key: %s"), *NewKey.ToString());
				SetKeyForAction(JumpAction, NewKey);
			}
			else if (Pair.Key == TEXT("Action") && SpecialAction)
			{
				UE_LOG(LogTemp, Warning, TEXT("Applying Action key: %s"), *NewKey.ToString());
				SetKeyForAction(SpecialAction, NewKey);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid key: %s"), *Pair.Value);
		}
	}
	
	// Apply the changes
	ApplyKeyRemapping();
}


