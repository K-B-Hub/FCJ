// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/RotationPlatform.h"

// Sets default values
ARotationPlatform::ARotationPlatform()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set default rotation speed (0 degrees per second for all axes)
	RotationSpeed = FRotator(0.0f, 0.0f, 0.0f);
}

// Called when the game starts or when spawned
void ARotationPlatform::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARotationPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Skip rotation if rotation speed is zero
	if (RotationSpeed.IsNearlyZero())
	{
		return;
	}

	// Calculate rotation delta for this frame
	FRotator DeltaRotation = RotationSpeed * DeltaTime;

	// Add rotation to current rotation
	AddActorWorldRotation(DeltaRotation);
}

