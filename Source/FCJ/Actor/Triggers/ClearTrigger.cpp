// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Triggers/ClearTrigger.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "Net/UnrealNetwork.h"

AClearTrigger::AClearTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// 레플리케이션 설정
	bReplicates = true;

	// 콜리전 박스 생성 (ZoneVolume과 오버랩 감지용)
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionBox->SetGenerateOverlapEvents(true);
	RootComponent = CollisionBox;

	// 트리거 컴포넌트 생성
	TriggerComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TriggerComponent"));
	TriggerComponent->SetupAttachment(RootComponent);

	bIsCleared = false;
	CachedTrigger = nullptr;
}

void AClearTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AClearTrigger, bIsCleared);
}

void AClearTrigger::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[ClearTrigger] %s BeginPlay - HasAuthority: %d"), *GetName(), HasAuthority());

	// 자식 액터 캐싱 및 델리게이트 바인딩
	if (TriggerComponent)
	{
		CachedTrigger = Cast<ABaseTrigger>(TriggerComponent->GetChildActor());
		if (CachedTrigger)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ClearTrigger] %s - CachedTrigger: %s"), *GetName(), *CachedTrigger->GetName());
			// 트리거 상태 변경 델리게이트 바인딩
			CachedTrigger->OnTriggerStateChanged.AddDynamic(this, &AClearTrigger::OnTriggerStateChangedCallback);
			UE_LOG(LogTemp, Warning, TEXT("[ClearTrigger] %s - Delegate bound to trigger"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ClearTrigger] %s - Failed to get child trigger actor!"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ClearTrigger] %s - TriggerComponent is null!"), *GetName());
	}
}

void AClearTrigger::OnRep_IsCleared()
{
	UE_LOG(LogTemp, Warning, TEXT("[ClearTrigger] %s OnRep_IsCleared - bIsCleared: %d"), *GetName(), bIsCleared);
	// 클라이언트에서 클리어 상태 변경 시 델리게이트 브로드캐스트
	OnClearStateChanged.Broadcast(bIsCleared);
}

void AClearTrigger::OnTriggerStateChangedCallback(bool bNewState)
{
	UE_LOG(LogTemp, Warning, TEXT("[ClearTrigger] %s OnTriggerStateChangedCallback - bNewState: %d, HasAuthority: %d, bIsCleared: %d"),
		*GetName(), bNewState, HasAuthority(), bIsCleared);

	// 서버에서만 클리어 상태 변경
	if (!HasAuthority())
		return;

	// 트리거가 활성화되고, 아직 클리어되지 않았다면
	if (bNewState && !bIsCleared)
	{
		bIsCleared = true;
		UE_LOG(LogTemp, Warning, TEXT("[ClearTrigger] %s - CLEARED! Broadcasting delegate"), *GetName());

		// 서버에서 델리게이트 브로드캐스트
		OnClearStateChanged.Broadcast(bIsCleared);

		// 한번 클리어되면 더 이상 델리게이트를 들을 필요가 없음
		if (CachedTrigger)
		{
			CachedTrigger->OnTriggerStateChanged.RemoveDynamic(this, &AClearTrigger::OnTriggerStateChangedCallback);
			UE_LOG(LogTemp, Warning, TEXT("[ClearTrigger] %s - Delegate unbound"), *GetName());
		}
	}
}
