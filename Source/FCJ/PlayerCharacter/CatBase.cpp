// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/CatBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Actor/WallJumpObject.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACatBase::ACatBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create SpringArm component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = SpringArmLength;
	SpringArmComponent->bUsePawnControlRotation = bUsePawnControlRotation;
	SpringArmComponent->bInheritPitch = bInheritPitch;
	SpringArmComponent->bInheritYaw = bInheritYaw;
	SpringArmComponent->bInheritRoll = bInheritRoll;
	SpringArmComponent->SetRelativeLocation(SpringArmOffset);
	SpringArmComponent->SetRelativeRotation(FRotator(SpringArmPitch, 0.0f, 0.0f));

	// Create Camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	// Create Special Action Box component
	SpecialActionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpecialActionBox"));
	SpecialActionBox->SetupAttachment(RootComponent);
	SpecialActionBox->SetBoxExtent(SpecialActionBoxExtent);
	SpecialActionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SpecialActionBox->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	SpecialActionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SpecialActionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	SpecialActionBox->SetGenerateOverlapEvents(true);
	SpecialActionBox->SetHiddenInGame(false); // 블루프린트에서 보이도록 설정 (투명하게 표시됨)
	
	// 박스를 캐릭터 앞쪽으로 위치시키기
	SpecialActionBox->SetRelativeLocation(FVector(SpecialActionBoxExtent.X, 0.0f, 0.0f));

	// Configure character movement
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	GetCharacterMovement()->AirControl = AirControl;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

// Called when the game starts or when spawned
void ACatBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Apply Blueprint settings when the game starts
	ApplyBlueprintSettings();
}

// Called every frame
void ACatBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Reset wall jump counter when on ground
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		CurrentWallJumpsInAir = 0;
	}
}

// Called to bind functionality to input
void ACatBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACatBase::ApplyBlueprintSettings()
{
	if (SpringArmComponent)
	{
		SpringArmComponent->TargetArmLength = SpringArmLength;
		SpringArmComponent->bUsePawnControlRotation = bUsePawnControlRotation;
		SpringArmComponent->bInheritPitch = bInheritPitch;
		SpringArmComponent->bInheritYaw = bInheritYaw;
		SpringArmComponent->bInheritRoll = bInheritRoll;
		SpringArmComponent->SetRelativeLocation(SpringArmOffset);
		SpringArmComponent->SetRelativeRotation(FRotator(SpringArmPitch, 0.0f, 0.0f));
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
		GetCharacterMovement()->JumpZVelocity = JumpVelocity;
		GetCharacterMovement()->AirControl = AirControl;
	}

	if (SpecialActionBox)
	{
		SpecialActionBox->SetBoxExtent(SpecialActionBoxExtent);
		SpecialActionBox->SetRelativeLocation(FVector(SpecialActionBoxExtent.X, 0.0f, 0.0f));
	}
}

AWallJumpObject* ACatBase::FindNearestWallJumpObject() const
{
	TArray<AActor*> WallJumpObjects;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWallJumpObject::StaticClass(), WallJumpObjects);

	AWallJumpObject* NearestWall = nullptr;
	float MinDistance = WallJumpDetectionRadius;

	for (AActor* Actor : WallJumpObjects)
	{
		if (AWallJumpObject* WallObject = Cast<AWallJumpObject>(Actor))
		{
			float Distance = FVector::Dist(GetActorLocation(), WallObject->GetActorLocation());
			if (Distance <= WallJumpDetectionRadius && Distance < MinDistance)
			{
				if (WallObject->CanWallJump(GetActorLocation()))
				{
					MinDistance = Distance;
					NearestWall = WallObject;
				}
			}
		}
	}

	return NearestWall;
}

bool ACatBase::CanPerformWallJump() const
{
	// Check if character is in air
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		return false;
	}

	// Check cooldown
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastWallJumpTime < WallJumpCooldown)
	{
		return false;
	}

	// Check wall jump limit
	if (CurrentWallJumpsInAir >= MaxWallJumpsInAir)
	{
		return false;
	}

	// Check if there's a wall nearby
	AWallJumpObject* NearestWall = FindNearestWallJumpObject();
	return NearestWall != nullptr;
}

void ACatBase::PerformWallJump()
{
	if (!CanPerformWallJump())
	{
		return;
	}

	AWallJumpObject* NearestWall = FindNearestWallJumpObject();
	if (!NearestWall)
	{
		return;
	}

	// Get jump direction from wall
	FVector JumpDirection = NearestWall->GetWallJumpDirection(GetActorLocation());

	// Apply wall jump velocity
	GetCharacterMovement()->Velocity = JumpDirection;

	// Update wall jump tracking
	LastWallJumpTime = GetWorld()->GetTimeSeconds();
	CurrentWallJumpsInAir++;

	// Play jump animation/effects here if needed
	UE_LOG(LogTemp, Warning, TEXT("Wall Jump Performed! Direction: %s"), *JumpDirection.ToString());
}

void ACatBase::Jump()
{
	// Try wall jump first if in air
	if (!GetCharacterMovement()->IsMovingOnGround() && CanPerformWallJump())
	{
		PerformWallJump();
	}
	else
	{
		// Normal jump
		Super::Jump();
	}
}

