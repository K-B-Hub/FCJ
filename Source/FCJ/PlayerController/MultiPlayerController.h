// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MultiPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS(BlueprintType, Blueprintable)
class FCJ_API AMultiPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMultiPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Enhanced Input components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// Individual Movement InputActions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveBackwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SpecialAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ESCAction;
	
	// ESC Menu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UESCWidget> ESCWidgetClass;

public:
	// Input settings - made public for SettingsWidget access
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	bool bInvertMouseY = false;

	// Zoom settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	float ZoomSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	float MinZoomDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	float MaxZoomDistance = 800.0f;

protected:
	// Individual movement functions
	void MoveForward(const FInputActionValue& Value);
	void MoveBackward(const FInputActionValue& Value);
	void MoveLeft(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	
	void Look(const FInputActionValue& Value);
	void Jump();
	void StopJumping();
	void PerformSpecialAction();
	void OnSpecialActionReleased();
	void Zoom(const FInputActionValue& Value);
	void OpenESCMenu();

private:
	// Current movement input values for each direction
	float ForwardInputValue = 0.0f;
	float BackwardInputValue = 0.0f;
	float LeftInputValue = 0.0f;
	float RightInputValue = 0.0f;
	
	// Helper function to apply combined movement
	void ApplyCombinedMovement();

public:
	// Blueprint callable functions
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMouseSensitivity(float NewSensitivity) { MouseSensitivity = NewSensitivity; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInvertMouseY(bool bInvert) { bInvertMouseY = bInvert; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetZoomSpeed(float NewZoomSpeed) { ZoomSpeed = NewZoomSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetZoomLimits(float MinDistance, float MaxDistance) { MinZoomDistance = MinDistance; MaxZoomDistance = MaxDistance; }

	// Settings persistence
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadInputSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveInputSettings();

	// Key remapping functions
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyKeyRemapping();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	TArray<FKey> GetDefaultKeysForAction(UInputAction* Action);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetKeyForAction(UInputAction* Action, const FKey& NewKey);

	// Getter functions for SettingsWidget
	UFUNCTION(BlueprintCallable, Category = "Settings")
	UInputMappingContext* GetDefaultMappingContext() const { return DefaultMappingContext; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	UInputAction* GetMoveForwardAction() const { return MoveForwardAction; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	UInputAction* GetMoveBackwardAction() const { return MoveBackwardAction; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	UInputAction* GetMoveLeftAction() const { return MoveLeftAction; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	UInputAction* GetMoveRightAction() const { return MoveRightAction; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	UInputAction* GetJumpAction() const { return JumpAction; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	UInputAction* GetSpecialActionInput() const { return SpecialAction; }

private:
	// Config file section name for saving settings
	static const FString InputSettingsSection;

	// Helper function to apply key mappings from config
	void ApplyKeyMappingsFromConfig(const TMap<FString, FString>& KeyMappings);

	UPROPERTY()
	class UESCWidget* ESCWidget;

	bool bIsESCMenuOpen;

	void ShowESCMenu();
	void HideESCMenu();

public:
	// ESC Menu functions
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ExitGame();

	// RPC for server to kick client back to main menu
	UFUNCTION(Client, Reliable, Category = "Network")
	void ClientReturnToMainMenu();
};
