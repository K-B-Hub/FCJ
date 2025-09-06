// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "WallJumpObject.generated.h"

UCLASS(Blueprintable)
class FCJ_API AWallJumpObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AWallJumpObject();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	float WallJumpForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	float WallJumpVerticalForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	float DetectionDistance;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	FVector GetWallJumpDirection(const FVector& PlayerLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	bool CanWallJump(const FVector& PlayerLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	FVector GetWallNormal(const FVector& PlayerLocation) const;
};
