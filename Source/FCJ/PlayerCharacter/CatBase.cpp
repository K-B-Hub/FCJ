// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/CatBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Actor/Objects/WallJumpObject.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/OverlapResult.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ACatBase::ACatBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
	bReplicates = true;

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

	// Create Parkour Lower Box component (하단 감지 - 오버랩되어야 함)
	ParkourLowerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ParkourLowerBox"));
	ParkourLowerBox->SetupAttachment(RootComponent);
	ParkourLowerBox->SetBoxExtent(FVector(40.0f, 40.0f, 20.0f)); // 가로, 세로, 높이
	ParkourLowerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ParkourLowerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	ParkourLowerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ParkourLowerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
	ParkourLowerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // 캐릭터와는 오버랩하지 않음
	ParkourLowerBox->SetGenerateOverlapEvents(true);
	ParkourLowerBox->SetHiddenInGame(false);
	ParkourLowerBox->SetRelativeLocation(FVector(80.0f, 0.0f, -10.0f)); // 캐릭터 앞쪽 하단

	// Create Parkour Upper Box component (상단 감지 - 오버랩되면 안됨)
	ParkourUpperBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ParkourUpperBox"));
	ParkourUpperBox->SetupAttachment(RootComponent);
	ParkourUpperBox->SetBoxExtent(FVector(40.0f, 40.0f, 20.0f)); // 가로, 세로, 높이
	ParkourUpperBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ParkourUpperBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	ParkourUpperBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ParkourUpperBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
	ParkourUpperBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // 캐릭터와는 오버랩하지 않음
	ParkourUpperBox->SetGenerateOverlapEvents(true);
	ParkourUpperBox->SetHiddenInGame(false);
	ParkourUpperBox->SetRelativeLocation(FVector(80.0f, 0.0f, 40.0f)); // 캐릭터 앞쪽 상단

	// Configure character movement
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	GetCharacterMovement()->AirControl = AirControl;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// 루트 모션 설정
	GetCharacterMovement()->bAllowPhysicsRotationDuringAnimRootMotion = true;
}

void ACatBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACatBase, bIsPerformingParkour);
	DOREPLIFETIME(ACatBase, CurrentParkourActor);
	DOREPLIFETIME(ACatBase, bIsMontageePlaying);
	DOREPLIFETIME(ACatBase, CurrentSpeedModifier);
}

// Called when the game starts or when spawned
void ACatBase::BeginPlay()
{
	Super::BeginPlay();

	// 루트 모션 관련 설정 (BeginPlay에서 안전하게 호출)
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		GetMesh()->SetNotifyRigidBodyCollision(true);
	}

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

	if (bTryingToParkour)
	{
		PerformParkour();
	}
}

// Called to bind functionality to input
void ACatBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACatBase::ServerSetMovementSpeed_Implementation(float Multiplier)
{
	if (Multiplier < 0.0f || Multiplier > 2.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid speed multiplier: %f"), Multiplier);
		return;
	}
	CurrentSpeedModifier = Multiplier;
	UpdateMovementSpeed(Multiplier);
}

void ACatBase::ApplySpeedModifier(float Multiplier)
{
	ServerSetMovementSpeed(Multiplier);
}

void ACatBase::OnRep_SpeedModifier()
{
	UpdateMovementSpeed(CurrentSpeedModifier);
}

void ACatBase::UpdateMovementSpeed(float Multiplier)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement)
	{
		Movement->MaxWalkSpeed = MovementSpeed * Multiplier;
		Movement->JumpZVelocity = JumpVelocity * Multiplier;
	}
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

}

AWallJumpObject* ACatBase::FindNearestWallJumpObject() const
{
	TArray<AActor*> WallJumpObjects;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWallJumpObject::StaticClass(), WallJumpObjects);

	AWallJumpObject* NearestWall = nullptr;
	float MinDistance = WallJumpDetectionRadius;

	FVector PlayerLocation = GetActorLocation();

	for (AActor* Actor : WallJumpObjects)
	{
		if (AWallJumpObject* WallObject = Cast<AWallJumpObject>(Actor))
		{
			// Use surface distance instead of center-to-center distance
			float Distance = WallObject->GetDistanceToSurface(PlayerLocation);

			if (Distance <= WallJumpDetectionRadius && Distance < MinDistance)
			{
				if (WallObject->CanWallJump(PlayerLocation))
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

	// Get character's current velocity to preserve momentum
	FVector CurrentVelocity = GetCharacterMovement()->Velocity;

	// Get jump direction from wall (now includes velocity for natural movement)
	FVector JumpDirection = NearestWall->GetWallJumpDirection(GetActorLocation(), CurrentVelocity);

	// If we're the server, execute directly; otherwise call server RPC
	if (HasAuthority())
	{
		ServerPerformWallJump_Implementation(JumpDirection);
	}
	else
	{
		ServerPerformWallJump(JumpDirection);
	}
}

void ACatBase::ServerPerformWallJump_Implementation(FVector JumpDirection)
{
	// Server authoritative wall jump logic
	if (!CanPerformWallJump())
	{
		return;
	}

	// Apply wall jump velocity on server
	GetCharacterMovement()->Velocity = JumpDirection;

	// Update wall jump tracking
	LastWallJumpTime = GetWorld()->GetTimeSeconds();
	CurrentWallJumpsInAir++;

	// Broadcast to all clients for visual effects
	MulticastPerformWallJump(JumpDirection);
}

void ACatBase::MulticastPerformWallJump_Implementation(FVector JumpDirection)
{
	// Client-side visual effects and audio
	// Here you can add particle effects, sounds, etc.
}

void ACatBase::Jump()
{
	// 파쿠르 중이면 점프 무시
	if (bIsPerformingParkour)
	{
		return;
	}

	// 파쿠르 시도 변수 on
	bTryingToParkour = true;

	// 파쿠르가 안되면 다른 동작들 시도
	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		// 공중에서는 벽점프 시도
		if (CanPerformWallJump())
		{
			PerformWallJump();
			return;
		}
	}
	else
	{
		// 지상에서는 일반 점프
		Super::Jump();
	}
}

void ACatBase::StopJumping()
{
	Super::StopJumping();

	//파쿠르 시도 변수 off
	bTryingToParkour = false;
}

AActor* ACatBase::DetectParkourTarget() const
{
	// 박스 오버랩 이벤트 강제 업데이트
	ParkourLowerBox->UpdateOverlaps();
	ParkourUpperBox->UpdateOverlaps();

	// 하단 박스에서 오버랩되는 액터들 찾기
	TArray<AActor*> LowerOverlappingActors;
	ParkourLowerBox->GetOverlappingActors(LowerOverlappingActors, AActor::StaticClass());

	// 상단 박스에서 오버랩되는 액터들 찾기
	TArray<AActor*> UpperOverlappingActors;
	ParkourUpperBox->GetOverlappingActors(UpperOverlappingActors, AActor::StaticClass());

	// 대안: 박스 위치에서 직접 오버랩 테스트
	if (LowerOverlappingActors.Num() == 0 && UpperOverlappingActors.Num() == 0)
	{
		FVector LowerBoxLocation = ParkourLowerBox->GetComponentLocation();
		FVector UpperBoxLocation = ParkourUpperBox->GetComponentLocation();
		FVector LowerBoxExtent = ParkourLowerBox->GetScaledBoxExtent();
		FVector UpperBoxExtent = ParkourUpperBox->GetScaledBoxExtent();

		// 하단 박스 위치에서 오버랩 테스트
		TArray<FOverlapResult> LowerOverlapResults;
		FCollisionQueryParams LowerQueryParams;
		LowerQueryParams.AddIgnoredActor(this);

		bool bLowerHit = GetWorld()->OverlapMultiByChannel(
			LowerOverlapResults,
			LowerBoxLocation,
			FQuat::Identity,
			ECC_WorldStatic,
			FCollisionShape::MakeBox(LowerBoxExtent),
			LowerQueryParams
		);

		// 상단 박스 위치에서도 오버랩 테스트
		TArray<FOverlapResult> UpperOverlapResults;
		FCollisionQueryParams UpperQueryParams;
		UpperQueryParams.AddIgnoredActor(this);

		bool bUpperHit = GetWorld()->OverlapMultiByChannel(
			UpperOverlapResults,
			UpperBoxLocation,
			FQuat::Identity,
			ECC_WorldStatic,
			FCollisionShape::MakeBox(UpperBoxExtent),
			UpperQueryParams
		);

		// 하단에는 있지만 상단에는 없는 액터 찾기
		if (bLowerHit)
		{
			for (const FOverlapResult& LowerResult : LowerOverlapResults)
			{
				if (!LowerResult.GetActor() || !LowerResult.GetActor()->FindComponentByClass<UStaticMeshComponent>())
					continue;

				// 이 액터가 상단에도 있는지 확인
				bool bFoundInUpper = false;
				if (bUpperHit)
				{
					for (const FOverlapResult& UpperResult : UpperOverlapResults)
					{
						if (UpperResult.GetActor() == LowerResult.GetActor())
						{
							bFoundInUpper = true;
							break;
						}
					}
				}

				if (!bFoundInUpper)
				{
					return LowerResult.GetActor();
				}
			}
		}
	}

	// 하단 박스에는 오버랩되지만 상단 박스에는 오버랩되지 않는 액터 찾기
	for (AActor* LowerActor : LowerOverlappingActors)
	{
		// StaticMeshComponent가 있는지 확인 (캐릭터는 콜리전 설정으로 이미 제외됨)
		if (!LowerActor->FindComponentByClass<UStaticMeshComponent>())
		{
			continue;
		}

		// 상단 박스에는 오버랩되지 않는지 확인
		bool bIsInUpperBox = false;
		for (AActor* UpperActor : UpperOverlappingActors)
		{
			if (UpperActor == LowerActor)
			{
				bIsInUpperBox = true;
				break;
			}
		}

		if (!bIsInUpperBox)
		{
			return LowerActor;
		}
	}

	return nullptr;
}

bool ACatBase::CanPerformParkour() const
{
	// 이미 파쿠르 중이거나 다른 몽타주 플레이 중이면 불가능
	if (bIsPerformingParkour || bIsMontageePlaying)
	{
		return false;
	}

	// 파쿠르 몽타주가 설정되어 있는지 확인
	if (!ParkourMontage)
	{
		return false;
	}

	// 전방에 파쿠르 가능한 오브젝트가 있는지 확인
	AActor* ParkourTarget = DetectParkourTarget();
	return ParkourTarget != nullptr;
}

void ACatBase::PerformParkour()
{
	if (!CanPerformParkour())
	{
		return;
	}

	AActor* ParkourTarget = DetectParkourTarget();
	if (!ParkourTarget)
	{
		return;
	}

	// If we're the server, execute directly; otherwise call server RPC
	if (HasAuthority())
	{
		ServerPerformParkour_Implementation(ParkourTarget);
	}
	else
	{
		ServerPerformParkour(ParkourTarget);
	}
}

void ACatBase::ServerPerformParkour_Implementation(AActor* ParkourTarget)
{
	// Server authoritative parkour logic
	if (!ParkourTarget || !CanPerformParkour())
	{
		return;
	}

	// Set parkour state (replicated)
	bIsPerformingParkour = true;
	CurrentParkourActor = ParkourTarget;

	// Rotate character towards target
	FVector DirectionToTarget = (ParkourTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	if (!DirectionToTarget.IsNearlyZero())
	{
		FRotator TargetRotation = DirectionToTarget.Rotation();
		SetActorRotation(FRotator(0.0f, TargetRotation.Yaw, 0.0f));
	}

	// Attach character to parkour target to maintain relative position
	// KeepWorld: 현재 월드 위치를 유지하면서 부모에 붙음
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
	AttachToActor(ParkourTarget, AttachRules);

	// Change to Flying mode for Z-axis root motion
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	// Broadcast to all clients to start parkour animation
	MulticastPerformParkour(ParkourTarget);
}

void ACatBase::MulticastPerformParkour_Implementation(AActor* ParkourTarget)
{
	if (!ParkourTarget || !ParkourMontage)
	{
		return;
	}

	// Play parkour animation on all clients
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		// Play montage with root motion
		float MontageLength = AnimInstance->Montage_Play(ParkourMontage);
		bIsMontageePlaying = true;

		UE_LOG(LogTemp, Warning, TEXT("[PARKOUR] Animation PLAYING - Duration=%.2fs"), MontageLength);

		// Set timer for montage completion (only on server)
		if (HasAuthority())
		{
			FTimerHandle ParkourTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				ParkourTimerHandle,
				this,
				&ACatBase::OnParkourMontageCompleted,
				MontageLength,
				false
			);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[PARKOUR] AnimInstance is NULL!"));
	}
}

void ACatBase::OnParkourMontageCompleted()
{
	// Only called on server
	if (!HasAuthority())
	{
		return;
	}

	// Detach from parkour target
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, false);
	DetachFromActor(DetachRules);

	// Reset parkour state (replicated)
	bIsPerformingParkour = false;
	CurrentParkourActor = nullptr;
	bIsMontageePlaying = false;

	// Restore Walking mode
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

FVector ACatBase::CalculateParkourStartLocation(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	FVector PlayerLocation = GetActorLocation();
	FVector ActorLocation = Actor->GetActorLocation();

	FVector DirectionToActor = (ActorLocation - PlayerLocation);
	DirectionToActor.Z = 0.0f;
	DirectionToActor.Normalize();

	FVector StartLocation = ActorLocation - (DirectionToActor * 30.0f);
	StartLocation.Z = PlayerLocation.Z;

	return StartLocation;
}

FVector ACatBase::CalculateParkourTargetLocation(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	FVector Origin, BoxExtent;
	Actor->GetActorBounds(false, Origin, BoxExtent);

	FVector TargetLocation = Origin;
	TargetLocation.Z = Origin.Z + BoxExtent.Z + 100.0f;

	return TargetLocation;
}

FVector ACatBase::CalculateParkourStartLocationPrecise(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	FVector ActorOrigin, ActorBoxExtent;
	Actor->GetActorBounds(false, ActorOrigin, ActorBoxExtent);

	FVector CharacterLocation = GetActorLocation();

	FVector DirectionToActor = (ActorOrigin - CharacterLocation);
	DirectionToActor.Z = 0.0f;
	DirectionToActor.Normalize();

	FVector2D ObjectSize2D = FVector2D(ActorBoxExtent.X, ActorBoxExtent.Y);
	float MaxExtent = FMath::Max(ObjectSize2D.X, ObjectSize2D.Y);
	FVector ObjectEdge = ActorOrigin - (DirectionToActor * MaxExtent);

	FVector StartLocation = ObjectEdge - (DirectionToActor * 30.0f);

	float ObjectTopZ = ActorOrigin.Z + ActorBoxExtent.Z;
	StartLocation.Z = ObjectTopZ - 50.0f;

	return StartLocation;
}

FVector ACatBase::CalculateParkourTargetLocationPrecise(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	FVector ActorOrigin, ActorBoxExtent;
	Actor->GetActorBounds(false, ActorOrigin, ActorBoxExtent);

	FVector CharacterLocation = GetActorLocation();
	FVector DirectionToActor = (ActorOrigin - CharacterLocation);
	DirectionToActor.Z = 0.0f;
	DirectionToActor.Normalize();

	FVector2D TargetSize2D = FVector2D(ActorBoxExtent.X, ActorBoxExtent.Y);
	float MaxTargetExtent = FMath::Max(TargetSize2D.X, TargetSize2D.Y);
	FVector TargetEdge = ActorOrigin + (DirectionToActor * MaxTargetExtent);

	FVector TargetLocation = TargetEdge;
	TargetLocation.Z = ActorOrigin.Z + ActorBoxExtent.Z + 10.0f;

	return TargetLocation;
}