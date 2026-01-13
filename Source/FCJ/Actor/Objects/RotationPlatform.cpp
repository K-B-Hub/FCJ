// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Objects/RotationPlatform.h"

// Sets default values
ARotationPlatform::ARotationPlatform()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	// Disable movement replication - all clients calculate rotation locally for smooth deterministic movement
	SetReplicateMovement(false);

	// Create mesh component (RootComponent, 실제 물리 담당)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	if (MeshComponent)
	{
		MeshComponent->SetIsReplicated(true);
	}

	// Set default rotation speed (0 degrees per second for all axes)
	RotationSpeed = FRotator(0.0f, 0.0f, 0.0f);
}

// Called when the game starts or when spawned
void ARotationPlatform::BeginPlay()
{
	Super::BeginPlay();

	// Ensure all attached actors and their components are movable
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (AActor* AttachedActor : AttachedActors)
	{
		if (AttachedActor)
		{
			// Set actor mobility to movable
			USceneComponent* RootComp = AttachedActor->GetRootComponent();
			if (RootComp && RootComp->Mobility != EComponentMobility::Movable)
			{
				RootComp->SetMobility(EComponentMobility::Movable);
				UE_LOG(LogTemp, Warning, TEXT("[RotationPlatform] Set %s root component to Movable"),
					*AttachedActor->GetName());
			}

			// Also set all static mesh components to movable
			TArray<UStaticMeshComponent*> MeshComponents;
			AttachedActor->GetComponents<UStaticMeshComponent>(MeshComponents);
			for (UStaticMeshComponent* MeshComp : MeshComponents)
			{
				if (MeshComp && MeshComp->Mobility != EComponentMobility::Movable)
				{
					MeshComp->SetMobility(EComponentMobility::Movable);
				}
			}
		}
	}
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

	// Add rotation to current rotation (local space)
	// All clients calculate rotation locally for smooth movement (matches MovingPlatform pattern)
	AddActorLocalRotation(DeltaRotation);
}

