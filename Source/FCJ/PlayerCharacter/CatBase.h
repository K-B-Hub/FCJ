// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CatBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class FCJ_API ACatBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACatBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Camera components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	// Blueprint configurable properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float SpringArmLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	FVector SpringArmOffset = FVector(0.0f, 0.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	float SpringArmPitch = -20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	bool bUsePawnControlRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	bool bInheritPitch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	bool bInheritYaw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
	bool bInheritRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float MovementSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float JumpVelocity = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Settings")
	float AirControl = 0.2f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Public getters for components
	UFUNCTION(BlueprintCallable, Category = "Camera")
	USpringArmComponent* GetSpringArmComponent() const { return SpringArmComponent; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	// Virtual functions for derived classes to override
	UFUNCTION(BlueprintImplementableEvent, Category = "Special Actions")
	void OnSpecialAction();

	UFUNCTION(BlueprintCallable, Category = "Special Actions")
	virtual void PerformSpecialAction() { OnSpecialAction(); }


private:
	// Apply Blueprint settings to components
	void ApplyBlueprintSettings();
};
