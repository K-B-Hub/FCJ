// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Objects/FadingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter/CatBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AFadingPlatform::AFadingPlatform()
{
	// Tick 비활성화 (타이머 기반 이벤트 처리 사용)
	PrimaryActorTick.bCanEverTick = false;

	// 네트워크 리플리케이션 활성화
	bReplicates = true;

	// 루트 컴포넌트로 StaticMesh 생성
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	RootComponent = PlatformMesh;

	// 발판 메시 충돌 설정
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformMesh->SetCollisionObjectType(ECC_WorldStatic);
	PlatformMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 트리거 박스 생성
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(PlatformMesh);
	TriggerBox->SetBoxExtent(FVector(100.0f, 100.0f, 50.0f));

	// 트리거 박스 충돌 설정 (Overlap만 감지)
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	// 기본값 설정
	FadeDelay = 1.0f;
	RespawnDelay = 3.0f;
	bAutoRespawn = false;
	bIsActive = true;
	bIsTriggered = false;
	bTriggerEnabled = true;
}

void AFadingPlatform::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 오버랩 이벤트 바인딩
	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFadingPlatform::OnTriggerBoxBeginOverlap);
	}

	// 초기 시각적 상태 설정
	UpdateVisualState();
}

void AFadingPlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFadingPlatform, bIsActive);
}

void AFadingPlatform::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	// 이미 트리거되었거나 비활성화 상태면 무시
	if (bIsTriggered || !bIsActive)
	{
		return;
	}

	// 트리거가 비활성화되어 있으면 무시
	if (!bTriggerEnabled)
	{
		return;
	}

	// 플레이어 캐릭터인지 확인
	ACatBase* PlayerCharacter = Cast<ACatBase>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Player stepped on platform: %s"), *GetName());

	// 트리거 상태로 변경
	bIsTriggered = true;

	// 델리게이트 브로드캐스트 (다른 액터들에게 알림)
	OnPlatformStepped.Broadcast(this);

	// FadeDelay 후에 발판 사라지기
	GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &AFadingPlatform::FadePlatform, FadeDelay, false);
}

void AFadingPlatform::FadePlatform()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Platform fading: %s"), *GetName());

	// 발판 비활성화
	bIsActive = false;

	// 서버에서도 시각적 상태 업데이트 (RepNotify는 서버에서 호출되지 않음)
	UpdateVisualState();

	// 자동 재생성 설정이 되어 있으면 타이머 시작
	if (bAutoRespawn)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AFadingPlatform::RespawnPlatform, RespawnDelay, false);
	}
}

void AFadingPlatform::RespawnPlatform()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Platform respawning: %s"), *GetName());

	// 발판 활성화
	bIsActive = true;
	bIsTriggered = false;

	// 서버에서도 시각적 상태 업데이트
	UpdateVisualState();

	// 타이머 정리
	GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
}

void AFadingPlatform::OnRep_IsActive()
{
	// 모든 클라이언트에서 시각적 상태 업데이트
	UpdateVisualState();
}

void AFadingPlatform::UpdateVisualState()
{
	if (!PlatformMesh)
	{
		return;
	}

	if (bIsActive)
	{
		// 발판 활성화: 충돌 및 가시성 켜기
		PlatformMesh->SetVisibility(true);
		PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Platform activated (visible): %s"), *GetName());
	}
	else
	{
		// 발판 비활성화: 충돌 및 가시성 끄기
		PlatformMesh->SetVisibility(false);
		PlatformMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Platform deactivated (invisible): %s"), *GetName());
	}
}

void AFadingPlatform::ResetPlatform()
{
	// 서버에서만 실행 (클라이언트에서 호출 시 무시)
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[FadingPlatform] ResetPlatform called on client - ignored"));
		return;
	}

	// 모든 타이머 정리
	GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	// 초기 상태로 리셋
	bIsActive = true;
	bIsTriggered = false;
	bTriggerEnabled = true;

	// 서버에서도 시각적 상태 업데이트
	UpdateVisualState();

	UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Platform reset: %s"), *GetName());
}

void AFadingPlatform::SetTriggerEnabled(bool bEnabled)
{
	bTriggerEnabled = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Trigger %s for platform: %s"),
		bEnabled ? TEXT("enabled") : TEXT("disabled"), *GetName());
}

void AFadingPlatform::ActivatePlatformWithoutTrigger()
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[FadingPlatform] ActivatePlatformWithoutTrigger called on client - ignored"));
		return;
	}

	// 발판을 보이게 하지만 트리거는 비활성화
	bIsActive = true;
	bIsTriggered = false;
	bTriggerEnabled = false;

	// 모든 타이머 정리
	GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	// 서버에서도 시각적 상태 업데이트
	UpdateVisualState();

	UE_LOG(LogTemp, Log, TEXT("[FadingPlatform] Platform activated without trigger: %s"), *GetName());
}
