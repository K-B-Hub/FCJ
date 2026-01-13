// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Objects/SpringTrap.h"
#include "PlayerCharacter/CatBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values
ASpringTrap::ASpringTrap()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Create base mesh (optional - can be set to nullptr in Blueprint)
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BaseMesh->SetCollisionObjectType(ECC_WorldStatic);
	BaseMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Create trap mesh (the moving part)
	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	TrapMesh->SetupAttachment(RootComponent);
	TrapMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TrapMesh->SetCollisionObjectType(ECC_WorldDynamic);
	TrapMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Create trigger box
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(TriggerBoxExtent);
	TriggerBox->SetRelativeLocation(TriggerBoxOffset);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	// Initialize state
	CurrentState = ETrapState::Retracted;
	bIsExtended = false;
	bCanTrigger = true;
}

// Called when the game starts or when spawned
void ASpringTrap::BeginPlay()
{
	Super::BeginPlay();

	// Enable actor and component replication for network synchronization
	SetReplicateMovement(true);

	if (RootSceneComponent)
	{
		RootSceneComponent->SetIsReplicated(true);
	}

	if (BaseMesh)
	{
		BaseMesh->SetIsReplicated(true);
	}

	if (TrapMesh)
	{
		TrapMesh->SetIsReplicated(true);
	}

	// Only bind overlap events on server (following FadingPlatform pattern)
	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(
			this, &ASpringTrap::OnTriggerBoxBeginOverlap
		);
	}

	// Store initial trap mesh position for interpolation
	InitialTrapLocation = TrapMesh->GetRelativeLocation();
	ExtendedTrapLocation = InitialTrapLocation + FVector(0.0f, 0.0f, ExtensionHeight);
}

void ASpringTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpringTrap, CurrentState);
	DOREPLIFETIME(ASpringTrap, bIsExtended);
	DOREPLIFETIME(ASpringTrap, bCanTrigger);
}

// Called every frame
void ASpringTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update mesh position based on current state
	UpdateTrapMeshPosition(DeltaTime);
}

// ===== Overlap 감지 =====

void ASpringTrap::OnTriggerBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Server authority check (redundant but safe)
	if (!HasAuthority())
	{
		return;
	}

	// Check if trap can be triggered
	if (!bCanTrigger || CurrentState != ETrapState::Retracted)
	{
		return;
	}

	// Validate actor is ACatBase
	ACatBase* Character = Cast<ACatBase>(OtherActor);
	if (!Character)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s triggered by %s"),
		*GetName(), *Character->GetName());

	// Activate trap
	ActivateTrap();
}

// ===== 상태 전환 함수 =====

void ASpringTrap::ActivateTrap()
{
	if (!HasAuthority() || CurrentState != ETrapState::Retracted)
	{
		return;
	}

	// Change state to extending
	CurrentState = ETrapState::Extending;
	bCanTrigger = false;

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s activating - launching characters"), *GetName());

	// Launch all overlapping characters
	TArray<AActor*> OverlappingActors;
	TriggerBox->GetOverlappingActors(OverlappingActors, ACatBase::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (ACatBase* Character = Cast<ACatBase>(Actor))
		{
			LaunchCharacter(Character);
		}
	}

	// Calculate extension duration based on height and speed
	float ExtensionDuration = ExtensionHeight / ExtensionSpeed;

	// Set timer for extension completion
	GetWorldTimerManager().SetTimer(
		ExtensionTimerHandle,
		this,
		&ASpringTrap::OnExtensionComplete,
		ExtensionDuration,
		false
	);
}

void ASpringTrap::OnExtensionComplete()
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s extension complete"), *GetName());

	CurrentState = ETrapState::Extended;
	bIsExtended = true;

	// Set timer for retraction
	GetWorldTimerManager().SetTimer(
		RetractTimerHandle,
		this,
		&ASpringTrap::StartRetraction,
		ExtendedHoldTime,
		false
	);
}

void ASpringTrap::StartRetraction()
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s starting retraction"), *GetName());

	CurrentState = ETrapState::Retracting;
	bIsExtended = false;

	// Calculate retraction duration
	float RetractionDuration = ExtensionHeight / RetractionSpeed;

	// Set timer for retraction completion
	GetWorldTimerManager().SetTimer(
		RetractionTimerHandle,
		this,
		&ASpringTrap::OnRetractionComplete,
		RetractionDuration,
		false
	);
}

void ASpringTrap::OnRetractionComplete()
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s retraction complete, entering cooldown"),
		*GetName());

	CurrentState = ETrapState::Cooldown;

	// Set cooldown timer
	GetWorldTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&ASpringTrap::OnCooldownComplete,
		CooldownTime,
		false
	);
}

void ASpringTrap::OnCooldownComplete()
{
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s cooldown complete, trap ready"),
		*GetName());

	CurrentState = ETrapState::Retracted;
	bCanTrigger = true;

	// Clear all timer handles
	GetWorldTimerManager().ClearTimer(ExtensionTimerHandle);
	GetWorldTimerManager().ClearTimer(RetractTimerHandle);
	GetWorldTimerManager().ClearTimer(RetractionTimerHandle);
	GetWorldTimerManager().ClearTimer(CooldownTimerHandle);
}

// ===== 발사 메커니즘 =====

void ASpringTrap::LaunchCharacter(ACatBase* Character)
{
	if (!Character || !HasAuthority())
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Calculate launch direction (forward vector + angle)
	FVector LaunchDirection = CalculateLaunchDirection();

	// Separate horizontal and vertical components (following Projectile pattern)
	FVector HorizontalComponent = FVector(
		LaunchDirection.X,
		LaunchDirection.Y,
		0.0f
	).GetSafeNormal() * LaunchForce;

	FVector VerticalComponent = FVector(0.0f, 0.0f, VerticalLaunchForce);

	FVector TotalLaunchImpulse = HorizontalComponent + VerticalComponent;

	// Apply impulse to character
	MovementComp->AddImpulse(TotalLaunchImpulse, true);

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] Launched %s with impulse %s"),
		*Character->GetName(), *TotalLaunchImpulse.ToString());
}

FVector ASpringTrap::CalculateLaunchDirection() const
{
	// Get trap's forward vector
	FVector ForwardVector = GetActorForwardVector();

	// Calculate direction with angle
	// Rotate forward vector upward by LaunchAngle
	FVector LaunchDir = ForwardVector.RotateAngleAxis(LaunchAngle, GetActorRightVector());

	return LaunchDir.GetSafeNormal();
}

// ===== 애니메이션 =====

void ASpringTrap::UpdateTrapMeshPosition(float DeltaTime)
{
	if (!TrapMesh)
	{
		return;
	}

	FVector TargetLocation;
	float InterpolationSpeed;

	switch (CurrentState)
	{
	case ETrapState::Extending:
		TargetLocation = ExtendedTrapLocation;
		InterpolationSpeed = ExtensionSpeed;
		break;

	case ETrapState::Extended:
		TargetLocation = ExtendedTrapLocation;
		InterpolationSpeed = 0.0f;  // No movement
		break;

	case ETrapState::Retracting:
	case ETrapState::Cooldown:
		TargetLocation = InitialTrapLocation;
		InterpolationSpeed = RetractionSpeed;
		break;

	case ETrapState::Retracted:
	default:
		TargetLocation = InitialTrapLocation;
		InterpolationSpeed = 0.0f;  // No movement
		break;
	}

	// Interpolate to target (following MovingDoor pattern)
	FVector CurrentLocation = TrapMesh->GetRelativeLocation();

	if (InterpolationSpeed > 0.0f && !CurrentLocation.Equals(TargetLocation, 1.0f))
	{
		FVector NewLocation = FMath::VInterpConstantTo(
			CurrentLocation,
			TargetLocation,
			DeltaTime,
			InterpolationSpeed
		);

		TrapMesh->SetRelativeLocation(NewLocation);
	}
	else if (InterpolationSpeed == 0.0f)
	{
		// Snap to exact position when not moving
		TrapMesh->SetRelativeLocation(TargetLocation);
	}
}

// ===== 네트워크 =====

void ASpringTrap::OnRep_TrapState()
{
	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s state replicated: %d"),
		*GetName(), static_cast<int32>(CurrentState));

	// Visual state updates happen automatically in Tick via UpdateTrapMeshPosition
	// No additional work needed here - Tick handles smooth interpolation on all clients
}

// ===== 유틸리티 =====

void ASpringTrap::ResetTrap()
{
	// Server authority only (following FadingPlatform pattern)
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SpringTrap] ResetTrap called on client - ignored"));
		return;
	}

	// Clear all timers
	GetWorldTimerManager().ClearTimer(ExtensionTimerHandle);
	GetWorldTimerManager().ClearTimer(RetractTimerHandle);
	GetWorldTimerManager().ClearTimer(RetractionTimerHandle);
	GetWorldTimerManager().ClearTimer(CooldownTimerHandle);

	// Reset to initial state
	CurrentState = ETrapState::Retracted;
	bIsExtended = false;
	bCanTrigger = true;

	// Snap trap mesh to retracted position
	if (TrapMesh)
	{
		TrapMesh->SetRelativeLocation(InitialTrapLocation);
	}

	UE_LOG(LogTemp, Log, TEXT("[SpringTrap] %s reset to initial state"), *GetName());
}
