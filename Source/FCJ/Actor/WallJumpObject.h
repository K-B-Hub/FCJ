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

	// Parkour/Climbing Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour", meta = (ToolTip = "이 오브젝트에서 파쿠르(클라이밍)가 가능한지 설정"))
	bool bCanParkour = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour", meta = (ToolTip = "파쿠르 완료 후 캐릭터가 도달할 목표 지점의 상대 위치"))
	FVector ParkourTargetOffset = FVector(0.0f, 0.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour", meta = (ToolTip = "파쿠르 애니메이션 시작 전 캐릭터가 이동할 시작 위치의 상대 오프셋"))
	FVector ParkourStartOffset = FVector(-50.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour", meta = (ToolTip = "파쿠르가 가능한 최대 거리"))
	float ParkourDetectionDistance = 150.0f;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	FVector GetWallJumpDirection(const FVector& PlayerLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	bool CanWallJump(const FVector& PlayerLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Wall Jump")
	FVector GetWallNormal(const FVector& PlayerLocation) const;

	// Parkour functions
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	bool CanParkour(const FVector& CharacterLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	FVector GetParkourStartLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	FVector GetParkourTargetLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Parkour")
	FVector GetParkourDirection() const;
};
