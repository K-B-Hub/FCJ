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
#include "Widdget/ESCWidget.h"
#include "InputModifiers.h"
#include "Subsystem/MultiSessionSubsystem.h"
#include "GameInstance/FCJGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

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
	
	// ESC Menu
	ESCWidget = nullptr;
	bIsESCMenuOpen = false;
}

void AMultiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only proceed with local player setup
	if (!IsLocalPlayerController())
	{
		return;
	}

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

	// Ensure proper input mode for gameplay (fix mouse cursor issue)
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
	UE_LOG(LogTemp, Warning, TEXT("MultiPlayerController BeginPlay: Set input mode to GameOnly"));

	// Load and apply display settings to ensure they persist across level changes
	if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings())
	{
		GameUserSettings->LoadSettings();
		GameUserSettings->ApplySettings(false);
		UE_LOG(LogTemp, Warning, TEXT("Loaded and applied display settings in game level"));
	}

	// Create ESC Widget once at BeginPlay
	if (ESCWidgetClass)
	{
		ESCWidget = CreateWidget<UESCWidget>(this, ESCWidgetClass);
		if (ESCWidget)
		{
			// Bind button callbacks once
			ESCWidget->OnResumeButtonClicked.AddDynamic(this, &AMultiPlayerController::ResumeGame);
			ESCWidget->OnMainMenuButtonClicked.AddDynamic(this, &AMultiPlayerController::ReturnToMainMenu);
			ESCWidget->OnExitGameButtonClicked.AddDynamic(this, &AMultiPlayerController::ExitGame);

			// Add to viewport but keep hidden initially
			ESCWidget->AddToViewport();
			ESCWidget->SetVisibility(ESlateVisibility::Hidden);

			UE_LOG(LogTemp, Warning, TEXT("ESC Widget created and added to viewport (hidden)"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create ESC Widget"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ESCWidgetClass is not set! Please assign it in Blueprint."));
	}

	// 스트리밍 완료 체크 시작 (모든 리소스가 로드되면 로딩 위젯 숨김)
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFCJGameInstance* FCJGameInstance = Cast<UFCJGameInstance>(GameInstance))
		{
			FCJGameInstance->StartCheckingStreamingCompletion();
		}
	}
}

void AMultiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent called for MultiPlayerController"));

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Enhanced Input Component found"));
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

		// ESC Menu
		if (ESCAction)
		{
			UE_LOG(LogTemp, Warning, TEXT("Binding ESCAction"));
			EnhancedInputComponent->BindAction(ESCAction, ETriggerEvent::Started, this, &AMultiPlayerController::OpenESCMenu);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ESCAction is null!"));
		}
	}
}


// Individual movement functions
void AMultiPlayerController::MoveForward(const FInputActionValue& Value)
{
	ForwardInputValue = Value.Get<float>();
	ApplyCombinedMovement();
}

void AMultiPlayerController::MoveBackward(const FInputActionValue& Value)
{
	BackwardInputValue = Value.Get<float>();
	ApplyCombinedMovement();
}

void AMultiPlayerController::MoveLeft(const FInputActionValue& Value)
{
	LeftInputValue = Value.Get<float>();
	ApplyCombinedMovement();
}

void AMultiPlayerController::MoveRight(const FInputActionValue& Value)
{
	RightInputValue = Value.Get<float>();
	ApplyCombinedMovement();
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
		if (ACatBase* character = Cast<ACatBase>(ControlledPawn))
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

	
	// Apply key mappings to input actions
	for (const auto& Pair : KeyMappings)
	{
		FKey NewKey(*Pair.Value);
		
		if (NewKey.IsValid())
		{
			// Individual Movement Actions
			if (Pair.Key == TEXT("MoveForward") && MoveForwardAction)
			{
				SetKeyForAction(MoveForwardAction, NewKey);
			}
			else if (Pair.Key == TEXT("MoveBackward") && MoveBackwardAction)
			{
				SetKeyForAction(MoveBackwardAction, NewKey);
			}
			else if (Pair.Key == TEXT("MoveLeft") && MoveLeftAction)
			{
				SetKeyForAction(MoveLeftAction, NewKey);
			}
			else if (Pair.Key == TEXT("MoveRight") && MoveRightAction)
			{
				SetKeyForAction(MoveRightAction, NewKey);
			}
			else if (Pair.Key == TEXT("Jump") && JumpAction)
			{
				SetKeyForAction(JumpAction, NewKey);
			}
			else if (Pair.Key == TEXT("Action") && SpecialAction)
			{
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

// ESC Menu Functions
void AMultiPlayerController::OpenESCMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("OpenESCMenu called, current state: %s"), bIsESCMenuOpen ? TEXT("Open") : TEXT("Closed"));
	
	if (bIsESCMenuOpen)
	{
		HideESCMenu();
	}
	else
	{
		ShowESCMenu();
	}
}


void AMultiPlayerController::ShowESCMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowESCMenu called"));
	
	if (ESCWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Showing ESC widget"));
		ESCWidget->SetVisibility(ESlateVisibility::Visible);
		
		// Don't pause the game in multiplayer - just show the menu
		// Show cursor and set input mode to game and UI
		SetShowMouseCursor(true);
		SetInputMode(FInputModeGameAndUI());
		
		bIsESCMenuOpen = true;
		UE_LOG(LogTemp, Warning, TEXT("ESC menu shown successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to show ESC menu - widget is null"));
	}
}

void AMultiPlayerController::HideESCMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("HideESCMenu called"));
	if (ESCWidget)
	{
		ESCWidget->SetVisibility(ESlateVisibility::Hidden);
		
		// Don't resume the game in multiplayer - game was never paused
		// Hide cursor and set input mode to game only
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
		
		bIsESCMenuOpen = false;
		UE_LOG(LogTemp, Warning, TEXT("ESC menu hidden successfully"));
	}
}

void AMultiPlayerController::ResumeGame()
{
	HideESCMenu();
}

void AMultiPlayerController::ReturnToMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ReturnToMainMenu called"));

	// Don't need to resume game in multiplayer - game was never paused
	// UGameplayStatics::SetGamePaused(GetWorld(), false);
	UMultiSessionSubsystem* Server = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
	if (Server)
	{
		// 레벨 전환 전에 즉시 bInServer를 false로 설정 (타이밍 문제 방지)
		Server->bInServer = false;

		// 클라이언트들 추방 및 세션 파괴
		Server->DestroyServer();
	}
	// Load main menu level
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}

void AMultiPlayerController::ExitGame()
{
	UE_LOG(LogTemp, Warning, TEXT("ExitGame called"));
	UMultiSessionSubsystem* Server = GetGameInstance()->GetSubsystem<UMultiSessionSubsystem>();
	if (Server)
	{
		// 게임 종료 전에 즉시 bInServer를 false로 설정 (세션 정리)
		Server->bInServer = false;

		// 클라이언트들 추방 및 세션 파괴
		Server->DestroyServer();
	}
	// Exit the game
	UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}

void AMultiPlayerController::ClientReturnToMainMenu_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("MultiPlayerController::ClientReturnToMainMenu - Client kicked from game"));

	// 세션 정보 정리
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiSessionSubsystem>();
		if (SessionSubsystem)
		{
			// 레벨 전환 전에 즉시 bInServer를 false로 설정 (타이밍 문제 방지)
			SessionSubsystem->bInServer = false;

			// 세션 정리 (비동기로 처리됨)
			SessionSubsystem->LeaveSession();

			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Client session cleared, returning to main menu"));
		}
	}

	// 메인 메뉴로 복귀
	UGameplayStatics::OpenLevel(this, FName("MainMenu"));
}


