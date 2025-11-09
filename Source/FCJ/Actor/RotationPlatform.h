// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RotationPlatform.generated.h"

UCLASS()
class FCJ_API ARotationPlatform : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARotationPlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Rotation speed for each axis (Roll, Pitch, Yaw) in degrees per second
	// X = Roll (rotation around X-axis), Y = Pitch (rotation around Y-axis), Z = Yaw (rotation around Z-axis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation Platform")
	FRotator RotationSpeed;

protected:
	// 메시 컴포넌트 (블루프린트에서 다양한 오브젝트 설정 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;
	
};
