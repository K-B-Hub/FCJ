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
	VelocityWeight = 0.3f;      // Character momentum weight (balanced default)
	WallNormalWeight = 0.7f;    // Wall push-off weight (slightly stronger)
}

void AWallJumpObject::BeginPlay()
{
	Super::BeginPlay();
}

void AWallJumpObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector AWallJumpObject::GetWallJumpDirection(const FVector& PlayerLocation, const FVector& CharacterVelocity) const
{
	// Get the wall's actual surface normal (now properly detects which wall face)
	FVector WallNormal = GetWallNormal(PlayerLocation);

	// Extract horizontal velocity (ignore Z axis for 2D plane calculation)
	FVector HorizontalVelocity = CharacterVelocity;
	HorizontalVelocity.Z = 0.0f;

	// Normalize velocity direction (safe normalization handles zero velocity)
	FVector VelocityDirection = HorizontalVelocity.GetSafeNormal();

	// Weighted blend: combine character's momentum with wall push-off
	// VelocityWeight controls how much character's movement is preserved
	// WallNormalWeight controls how much wall pushes character away
	FVector JumpDirection = (VelocityDirection * VelocityWeight + WallNormal * WallNormalWeight).GetSafeNormal();

	// Add vertical component for upward jump
	JumpDirection.Z = WallJumpVerticalForce / WallJumpForce;

	return JumpDirection * WallJumpForce;
}

float AWallJumpObject::GetDistanceToSurface(const FVector& Point) const
{
	if (!CollisionComponent)
	{
		// Fallback to center point distance if no collision component
		return FVector::Dist(GetActorLocation(), Point);
	}

	// Get the box extent and transform
	FVector BoxExtent = CollisionComponent->GetScaledBoxExtent();
	FTransform BoxTransform = CollisionComponent->GetComponentTransform();

	// Transform point to local space (handles rotation)
	FVector LocalPoint = BoxTransform.InverseTransformPosition(Point);

	// Create local space box centered at origin
	FBox LocalBox(-BoxExtent, BoxExtent);

	// Find closest point on the box surface (in local space)
	FVector ClosestPointLocal = LocalBox.GetClosestPointTo(LocalPoint);

	// Transform back to world space
	FVector ClosestPointWorld = BoxTransform.TransformPosition(ClosestPointLocal);

	// Calculate distance from point to closest point on box surface
	return FVector::Dist(ClosestPointWorld, Point);
}

bool AWallJumpObject::CanWallJump(const FVector& PlayerLocation) const
{
	float Distance = GetDistanceToSurface(PlayerLocation);
	return Distance <= DetectionDistance;
}

FVector AWallJumpObject::GetWallNormal(const FVector& PlayerLocation) const
{
	if (!CollisionComponent)
	{
		// Fallback: simple direction from wall to player
		FVector DirectionToPlayer = (PlayerLocation - GetActorLocation());
		DirectionToPlayer.Z = 0.0f;
		return DirectionToPlayer.GetSafeNormal();
	}

	// Get box extent and transform
	FVector BoxExtent = CollisionComponent->GetScaledBoxExtent();
	FTransform BoxTransform = CollisionComponent->GetComponentTransform();

	// Transform player location to local space
	FVector LocalPlayerPos = BoxTransform.InverseTransformPosition(PlayerLocation);
	LocalPlayerPos.Z = 0.0f; // Only consider horizontal plane

	// Handle edge case: player at box center
	if (FMath::Abs(LocalPlayerPos.X) < KINDA_SMALL_NUMBER && FMath::Abs(LocalPlayerPos.Y) < KINDA_SMALL_NUMBER)
	{
		// Default to +X face
		return BoxTransform.TransformVectorNoScale(FVector(1.0f, 0.0f, 0.0f)).GetSafeNormal();
	}

	// Normalize position by box extent to determine which face region player is in
	// This handles thin/long boxes correctly
	float NormalizedX = FMath::Abs(LocalPlayerPos.X) / FMath::Max(BoxExtent.X, KINDA_SMALL_NUMBER);
	float NormalizedY = FMath::Abs(LocalPlayerPos.Y) / FMath::Max(BoxExtent.Y, KINDA_SMALL_NUMBER);

	FVector LocalNormal;

	// Player is closer to X face if normalized X is greater
	if (NormalizedX > NormalizedY)
	{
		// X face: determine +X or -X based on sign
		LocalNormal = FVector(FMath::Sign(LocalPlayerPos.X), 0.0f, 0.0f);
	}
	else
	{
		// Y face: determine +Y or -Y based on sign
		LocalNormal = FVector(0.0f, FMath::Sign(LocalPlayerPos.Y), 0.0f);
	}

	// Transform local normal to world space
	FVector WorldNormal = BoxTransform.TransformVectorNoScale(LocalNormal);
	WorldNormal.Z = 0.0f;
	WorldNormal.Normalize();

	return WorldNormal;
}

bool AWallJumpObject::CanParkour(const FVector& CharacterLocation) const
{
	if (!bCanParkour)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), CharacterLocation);
	return Distance <= ParkourDetectionDistance;
}

/*FVector AWallJumpObject::GetParkourStartLocation() const
{
	return GetActorLocation() + GetActorTransform().TransformVectorNoScale(ParkourStartOffset);
}

FVector AWallJumpObject::GetParkourTargetLocation() const
{
	return GetActorLocation() + GetActorTransform().TransformVectorNoScale(ParkourTargetOffset);
}

FVector AWallJumpObject::GetParkourDirection() const
{
	FVector StartLocation = GetParkourStartLocation();
	FVector TargetLocation = GetParkourTargetLocation();
	
	FVector Direction = (TargetLocation - StartLocation).GetSafeNormal();
	return Direction;
}*/

