// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/CatBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Actor/WallJumpObject.h"
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

	// Call server RPC to perform wall jump
	ServerPerformWallJump(JumpDirection);
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

	UE_LOG(LogTemp, Warning, TEXT("Server: Wall Jump Performed! Direction: %s"), *JumpDirection.ToString());

	// Broadcast to all clients for visual effects
	MulticastPerformWallJump(JumpDirection);
}

void ACatBase::MulticastPerformWallJump_Implementation(FVector JumpDirection)
{
	// Client-side visual effects and audio
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client: Wall Jump visual effects! Direction: %s"), *JumpDirection.ToString());
		// Here you can add particle effects, sounds, etc.
	}
}

void ACatBase::Jump()
{
	UE_LOG(LogTemp, Warning, TEXT("Jump called - IsPerformingParkour: %s, IsOnGround: %s"), 
		bIsPerformingParkour ? TEXT("true") : TEXT("false"),
		GetCharacterMovement()->IsMovingOnGround() ? TEXT("true") : TEXT("false"));

	// 파쿠르 중이면 점프 무시
	if (bIsPerformingParkour)
	{
		UE_LOG(LogTemp, Warning, TEXT("Jump ignored - already performing parkour"));
		return;
	}

	// 지상과 공중 모두에서 파쿠르 가능
	if (CanPerformParkour())
	{
		if (GetCharacterMovement()->IsMovingOnGround())
		{
			UE_LOG(LogTemp, Warning, TEXT("Attempting parkour from ground - climb up"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Attempting parkour in air - grab and climb up"));
		}
		PerformParkour();
		return;
	}

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
		UE_LOG(LogTemp, Warning, TEXT("Performing normal jump from ground"));
		Super::Jump();
	}
}

AActor* ACatBase::DetectParkourTarget() const
{
	UE_LOG(LogTemp, Warning, TEXT("DetectParkourTarget called - using box overlap detection"));
	
	// 박스 위치 정보 출력
	FVector LowerBoxLocation = ParkourLowerBox->GetComponentLocation();
	FVector UpperBoxLocation = ParkourUpperBox->GetComponentLocation();
	FVector LowerBoxExtent = ParkourLowerBox->GetScaledBoxExtent();
	FVector UpperBoxExtent = ParkourUpperBox->GetScaledBoxExtent();
	
	UE_LOG(LogTemp, Warning, TEXT("Lower box location: %s, extent: %s"), 
		*LowerBoxLocation.ToString(), *LowerBoxExtent.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Upper box location: %s, extent: %s"), 
		*UpperBoxLocation.ToString(), *UpperBoxExtent.ToString());
	
	// 박스 오버랩 이벤트 강제 업데이트
	ParkourLowerBox->UpdateOverlaps();
	ParkourUpperBox->UpdateOverlaps();
	
	// 하단 박스에서 오버랩되는 액터들 찾기
	TArray<AActor*> LowerOverlappingActors;
	ParkourLowerBox->GetOverlappingActors(LowerOverlappingActors, AActor::StaticClass());
	
	// 상단 박스에서 오버랩되는 액터들 찾기
	TArray<AActor*> UpperOverlappingActors;
	ParkourUpperBox->GetOverlappingActors(UpperOverlappingActors, AActor::StaticClass());
	
	UE_LOG(LogTemp, Warning, TEXT("Lower box overlapping actors: %d, Upper box overlapping actors: %d"), 
		LowerOverlappingActors.Num(), UpperOverlappingActors.Num());
	
	// 각 오버랩된 액터들의 이름 출력
	for (int32 i = 0; i < LowerOverlappingActors.Num(); i++)
	{
		UE_LOG(LogTemp, Log, TEXT("Lower box actor %d: %s"), i, *LowerOverlappingActors[i]->GetName());
	}
	for (int32 i = 0; i < UpperOverlappingActors.Num(); i++)
	{
		UE_LOG(LogTemp, Log, TEXT("Upper box actor %d: %s"), i, *UpperOverlappingActors[i]->GetName());
	}
	
	// 대안: 박스 위치에서 직접 오버랩 테스트
	if (LowerOverlappingActors.Num() == 0 && UpperOverlappingActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No overlaps detected - trying alternative method with collision queries"));
		
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
		
		UE_LOG(LogTemp, Warning, TEXT("Direct overlap query - Lower: %s (%d results), Upper: %s (%d results)"), 
			bLowerHit ? TEXT("true") : TEXT("false"), LowerOverlapResults.Num(),
			bUpperHit ? TEXT("true") : TEXT("false"), UpperOverlapResults.Num());
		
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
					UE_LOG(LogTemp, Warning, TEXT("Valid parkour target via direct query: %s (in lower but not in upper)"), 
						*LowerResult.GetActor()->GetName());
					return LowerResult.GetActor();
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("Actor %s found in both upper and lower - not suitable for parkour"), 
						*LowerResult.GetActor()->GetName());
				}
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("No valid parkour target found via direct queries"));
	}
	
	// 하단 박스에는 오버랩되지만 상단 박스에는 오버랩되지 않는 액터 찾기
	for (AActor* LowerActor : LowerOverlappingActors)
	{
		// StaticMeshComponent가 있는지 확인 (캐릭터는 콜리전 설정으로 이미 제외됨)
		if (!LowerActor->FindComponentByClass<UStaticMeshComponent>())
		{
			UE_LOG(LogTemp, Log, TEXT("Actor %s has no StaticMeshComponent"), *LowerActor->GetName());
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
			UE_LOG(LogTemp, Warning, TEXT("Valid parkour target found: %s (in lower box but not in upper box)"), 
				*LowerActor->GetName());
			return LowerActor;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Actor %s is in both boxes - not suitable for parkour"), 
				*LowerActor->GetName());
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("No valid parkour target detected - need object in lower box but not in upper box"));
	return nullptr;
}

bool ACatBase::CanPerformParkour() const
{
	UE_LOG(LogTemp, Warning, TEXT("CanPerformParkour called"));

	// 이미 파쿠르 중이거나 다른 몽타주 플레이 중이면 불가능
	if (bIsPerformingParkour || bIsMontageePlaying)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanPerformParkour: false - already performing parkour or playing montage"));
		return false;
	}

	// 파쿠르 몽타주가 설정되어 있는지 확인
	if (!ParkourMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanPerformParkour: false - ParkourMontage not set"));
		return false;
	}

	// 전방에 파쿠르 가능한 오브젝트가 있는지 확인
	AActor* ParkourTarget = DetectParkourTarget();
	bool bResult = ParkourTarget != nullptr;
	UE_LOG(LogTemp, Warning, TEXT("CanPerformParkour: %s"), bResult ? TEXT("true") : TEXT("false"));
	return bResult;
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

	// Call server RPC to perform parkour
	ServerPerformParkour(ParkourTarget);
}

void ACatBase::ServerPerformParkour_Implementation(AActor* ParkourTarget)
{
	// Server authoritative parkour logic
	if (!ParkourTarget || !CanPerformParkour())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Server: Starting parkour on target: %s"), *ParkourTarget->GetName());

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

	// Change to Flying mode for Z-axis root motion
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	UE_LOG(LogTemp, Warning, TEXT("Server: Movement mode changed to Flying for Z-axis root motion"));

	// Broadcast to all clients to start parkour animation
	MulticastPerformParkour(ParkourTarget);
}

void ACatBase::MulticastPerformParkour_Implementation(AActor* ParkourTarget)
{
	if (!ParkourTarget || !ParkourMontage)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Multicast: Starting parkour animation on target: %s"), *ParkourTarget->GetName());

	// Play parkour animation on all clients
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting parkour animation at: %s"), *GetActorLocation().ToString());

		// Play montage with root motion
		float MontageLength = AnimInstance->Montage_Play(ParkourMontage);
		bIsMontageePlaying = true;

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

		UE_LOG(LogTemp, Warning, TEXT("Parkour animation started! Duration: %f seconds"), MontageLength);
	}
}

void ACatBase::OnParkourMontageCompleted()
{
	// Only called on server
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Server: Parkour Animation Completed!"));
	UE_LOG(LogTemp, Warning, TEXT("Final location after root motion: %s"), *GetActorLocation().ToString());

	// Reset parkour state (replicated)
	bIsPerformingParkour = false;
	CurrentParkourActor = nullptr;
	bIsMontageePlaying = false;

	// Restore Walking mode
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	UE_LOG(LogTemp, Warning, TEXT("Server: Parkour completed - movement mode restored to Walking"));
}

FVector ACatBase::CalculateParkourStartLocation(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	FVector PlayerLocation = GetActorLocation();
	FVector ActorLocation = Actor->GetActorLocation();
	
	// 플레이어에서 액터 방향으로 벡터 계산
	FVector DirectionToActor = (ActorLocation - PlayerLocation);
	DirectionToActor.Z = 0.0f; // 수평 방향만 고려
	DirectionToActor.Normalize();

	// 액터로부터 조금 떨어진 지점을 시작점으로 설정 (30 유닛)
	FVector StartLocation = ActorLocation - (DirectionToActor * 30.0f);
	StartLocation.Z = PlayerLocation.Z; // 플레이어와 같은 높이에서 시작

	UE_LOG(LogTemp, Warning, TEXT("CalculateParkourStartLocation - Player: %s, Actor: %s, Start: %s"), 
		*PlayerLocation.ToString(), *ActorLocation.ToString(), *StartLocation.ToString());

	return StartLocation;
}

FVector ACatBase::CalculateParkourTargetLocation(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	// 액터의 바운딩 박스 계산
	FVector Origin, BoxExtent;
	Actor->GetActorBounds(false, Origin, BoxExtent);

	// 액터 위쪽으로 목표 지점 설정 (표면에서 약간 위로)
	FVector TargetLocation = Origin;
	TargetLocation.Z = Origin.Z + BoxExtent.Z + 100.0f;

	UE_LOG(LogTemp, Warning, TEXT("CalculateParkourTargetLocation - Actor: %s, Origin: %s, BoxExtent: %s, Target: %s"), 
		*Actor->GetName(), *Origin.ToString(), *BoxExtent.ToString(), *TargetLocation.ToString());

	return TargetLocation;
}

FVector ACatBase::CalculateParkourStartLocationPrecise(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	// 오브젝트의 바운딩 박스 계산
	FVector ActorOrigin, ActorBoxExtent;
	Actor->GetActorBounds(false, ActorOrigin, ActorBoxExtent);

	// 캐릭터의 현재 위치
	FVector CharacterLocation = GetActorLocation();
	
	// 캐릭터에서 오브젝트로의 방향 계산
	FVector DirectionToActor = (ActorOrigin - CharacterLocation);
	DirectionToActor.Z = 0.0f; // 수평 방향만 고려
	DirectionToActor.Normalize();

	// 오브젝트의 가장자리 (캐릭터쪽 면) 계산
	FVector2D ObjectSize2D = FVector2D(ActorBoxExtent.X, ActorBoxExtent.Y);
	float MaxExtent = FMath::Max(ObjectSize2D.X, ObjectSize2D.Y);
	FVector ObjectEdge = ActorOrigin - (DirectionToActor * MaxExtent);
	
	// 캐릭터가 서야 할 위치: 오브젝트 가장자리에서 조금 떨어진 곳
	FVector StartLocation = ObjectEdge - (DirectionToActor * 30.0f); // 30 유닛 떨어짐
	
	// 높이는 오브젝트 상단과 캐릭터 허리가 맞도록 조정
	float ObjectTopZ = ActorOrigin.Z + ActorBoxExtent.Z; // 오브젝트 상단
	StartLocation.Z = ObjectTopZ - 50.0f; // 허리 높이 (오브젝트 상단에서 50 유닛 아래)

	UE_LOG(LogTemp, Warning, TEXT("CalculateParkourStartLocationPrecise - ObjectEdge: %s, StartLocation: %s"), 
		*ObjectEdge.ToString(), *StartLocation.ToString());

	return StartLocation;
}

FVector ACatBase::CalculateParkourTargetLocationPrecise(AActor* Actor) const
{
	if (!Actor)
	{
		return GetActorLocation();
	}

	// 오브젝트의 바운딩 박스 계산
	FVector ActorOrigin, ActorBoxExtent;
	Actor->GetActorBounds(false, ActorOrigin, ActorBoxExtent);

	// 캐릭터에서 오브젝트로의 방향 계산
	FVector CharacterLocation = GetActorLocation();
	FVector DirectionToActor = (ActorOrigin - CharacterLocation);
	DirectionToActor.Z = 0.0f;
	DirectionToActor.Normalize();

	// 오브젝트의 반대편 가장자리 (캐릭터가 최종적으로 서야 할 곳)
	FVector2D TargetSize2D = FVector2D(ActorBoxExtent.X, ActorBoxExtent.Y);
	float MaxTargetExtent = FMath::Max(TargetSize2D.X, TargetSize2D.Y);
	FVector TargetEdge = ActorOrigin + (DirectionToActor * MaxTargetExtent);
	
	// 오브젝트 위에 안전하게 서있을 위치
	FVector TargetLocation = TargetEdge;
	TargetLocation.Z = ActorOrigin.Z + ActorBoxExtent.Z + 10.0f; // 오브젝트 상단에서 10 유닛 위

	UE_LOG(LogTemp, Warning, TEXT("CalculateParkourTargetLocationPrecise - TargetEdge: %s, FinalTarget: %s"), 
		*TargetEdge.ToString(), *TargetLocation.ToString());

	return TargetLocation;
}

