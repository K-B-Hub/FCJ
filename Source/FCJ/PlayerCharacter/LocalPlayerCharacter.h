// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "LocalPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UChildActorComponent;
class AFrontCat;
class ABackCat;

UCLASS()
class FCJ_API ALocalPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALocalPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Character Class")
	TSubclassOf<AFrontCat> FrontCatClass;

	UPROPERTY(EditAnywhere, Category = "Character Class")
	TSubclassOf<ABackCat> BackCatClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	AFrontCat* FrontCat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ABackCat* BackCat;

	UFUNCTION(BlueprintPure)
	AFrontCat* GetFrontCat() const;

	UFUNCTION(BlueprintPure)
	ABackCat* GetBackCat() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	void HandleBackCatMovement(const FInputActionValue& Value);
	void HandleFrontCatMovement(const FInputActionValue& Value);
};
