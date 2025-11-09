// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/MovingPlatform.h"

// Sets default values
AMovingPlatform::AMovingPlatform()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	// Create mesh component (RootComponent, 실제 물리 담당)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Set default values
	MovementDistance = FVector(0.0f, 0.0f, 0.0f);
	MovementSpeed = 100.0f;
	MovementDirection = 1.0f;
}

// Called when the game starts or when spawned
void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);

	if (MeshComponent)
	{
		MeshComponent->SetIsReplicated(true);
	}
	
	// Store the starting location
	StartLocation = GetActorLocation();

	// Calculate target location
	TargetLocation = StartLocation + MovementDistance;
}

// Called every frame
void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Skip movement if speed or distance is zero
	if (MovementSpeed <= 0.0f || MovementDistance.IsNearlyZero())
	{
		return;
	}

	// Get current location
	FVector CurrentLocation = GetActorLocation();

	// Determine current target based on movement direction
	FVector CurrentTarget = (MovementDirection > 0.0f) ? TargetLocation : StartLocation;

	// Move towards the current target
	FVector NewLocation = FMath::VInterpConstantTo(
		CurrentLocation,
		CurrentTarget,
		DeltaTime,
		MovementSpeed
	);

	SetActorLocation(NewLocation);

	// Check if we've reached the target (with small tolerance)
	float DistanceToTarget = FVector::Dist(NewLocation, CurrentTarget);
	if (DistanceToTarget < 1.0f)
	{
		// Reverse direction
		MovementDirection *= -1.0f;
	}
}

