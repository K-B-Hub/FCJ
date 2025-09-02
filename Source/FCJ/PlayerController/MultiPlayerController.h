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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SpecialAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomAction;

	// Input settings
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

	// Input callback functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();
	void StopJumping();
	void PerformSpecialAction();
	void Zoom(const FInputActionValue& Value);

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
};
