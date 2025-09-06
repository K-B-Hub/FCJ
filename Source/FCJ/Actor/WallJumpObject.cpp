// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/WallJumpObject.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"

AWallJumpObject::AWallJumpObject()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetBoxExtent(FVector(50.0f, 50.0f, 100.0f));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Default values
	WallJumpForce = 500.0f;
	WallJumpVerticalForce = 400.0f;
	DetectionDistance = 150.0f;
}

void AWallJumpObject::BeginPlay()
{
	Super::BeginPlay();
}

void AWallJumpObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector AWallJumpObject::GetWallJumpDirection(const FVector& PlayerLocation) const
{
	FVector WallNormal = GetWallNormal(PlayerLocation);
	FVector JumpDirection = WallNormal;
	JumpDirection.Z = 0.0f;
	JumpDirection.Normalize();
	
	// Add vertical component
	JumpDirection.Z = WallJumpVerticalForce / WallJumpForce;
	
	return JumpDirection * WallJumpForce;
}

bool AWallJumpObject::CanWallJump(const FVector& PlayerLocation) const
{
	float Distance = FVector::Dist(GetActorLocation(), PlayerLocation);
	return Distance <= DetectionDistance;
}

FVector AWallJumpObject::GetWallNormal(const FVector& PlayerLocation) const
{
	FVector DirectionToPlayer = (PlayerLocation - GetActorLocation());
	DirectionToPlayer.Z = 0.0f;
	DirectionToPlayer.Normalize();
	
	// Return the normal pointing away from the wall (towards the player)
	return DirectionToPlayer;
}

