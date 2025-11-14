// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

UCLASS()
class FCJ_API AMovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMovingPlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Blueprint-editable movement distance for X, Y, Z axes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform")
	FVector MovementDistance;

	// Movement speed (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Platform")
	float MovementSpeed;

protected:
	// 메시 컴포넌트 (블루프린트에서 다양한 오브젝트 설정 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;
	
private:
	// Starting location of the platform
	FVector StartLocation;

	// Target location (StartLocation + MovementDistance)
	FVector TargetLocation;

	// Current movement direction (1.0 = forward, -1.0 = backward)
	float MovementDirection;

};
