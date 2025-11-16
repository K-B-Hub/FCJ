// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Volumes/RespawnVolume.h"
#include "PlayerCharacter/CatBase.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ARespawnVolume::ARespawnVolume()
{
	// Tick 비활성화 (오버랩 이벤트만 사용)
	PrimaryActorTick.bCanEverTick = false;

	// 멀티플레이어 복제 활성화
	bReplicates = true;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// 박스 컴포넌트 생성 및 설정
	RespawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RespawnBox"));
	RespawnBox->SetupAttachment(RootComponent);
	RespawnBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RespawnBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	RespawnBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 기본 박스 크기 설정
	RespawnBox->SetBoxExtent(FVector(1000.0f, 1000.0f, 100.0f));

	// 스폰 포인트 시각화 캡슐 생성 (독립적으로 RootComponent에 attach)
	SpawnPointVisualizer = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SpawnPointVisualizer"));
	SpawnPointVisualizer->SetupAttachment(RootComponent);

	// 캐릭터와 비슷한 캡슐 크기 설정
	SpawnPointVisualizer->SetCapsuleHalfHeight(96.0f);
	SpawnPointVisualizer->SetCapsuleRadius(42.0f);

	// 충돌 비활성화 (시각화 용도만)
	SpawnPointVisualizer->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 게임에서는 숨김, 에디터에서만 보임
	SpawnPointVisualizer->SetHiddenInGame(true);
	SpawnPointVisualizer->SetVisibility(true);

	// 기본 스폰 포인트 위치 (액터 위치 기준 위쪽)
	SpawnPointVisualizer->SetRelativeLocation(FVector(0.0f, 0.0f, 500.0f));
}

// Called when the game starts or when spawned
void ARespawnVolume::BeginPlay()
{
	Super::BeginPlay();

	// 오버랩 이벤트 바인딩
	RespawnBox->OnComponentBeginOverlap.AddDynamic(this, &ARespawnVolume::OnOverlapBegin);
}

void ARespawnVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 리스폰 처리
	if (!HasAuthority())
	{
		return;
	}

	// CatBase 캐릭터인지 확인
	ACatBase* Cat = Cast<ACatBase>(OtherActor);
	if (Cat && SpawnPointVisualizer)
	{
		// 시각화 캡슐의 월드 위치를 스폰 포인트로 사용
		FVector WorldSpawnPoint = SpawnPointVisualizer->GetComponentLocation();

		// 캐릭터를 스폰 포인트로 텔레포트
		Cat->SetActorLocation(WorldSpawnPoint, false, nullptr, ETeleportType::TeleportPhysics);

		// 캐릭터의 속도 초기화 (낙하 속도 제거)
		if (UCharacterMovementComponent* MovementComp = Cat->GetCharacterMovement())
		{
			MovementComp->Velocity = FVector::ZeroVector;
		}
	}
}

