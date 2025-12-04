// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/TriggeredTurret.h"
#include "Actor/TriggeredActors/Turret.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "Components/ChildActorComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"

ATriggeredTurret::ATriggeredTurret()
{
	// 델리게이트 기반 이벤트 처리를 사용하므로 Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Turret 자식 액터 컴포넌트 생성
	TurretComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("TurretComponent"));
	TurretComponent->SetupAttachment(RootSceneComponent);
	// Turret 클래스는 Blueprint에서 설정 (Child Actor Class)

	// 트리거 감지용 박스 컴포넌트 생성
	TriggerDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerDetectionBox"));
	TriggerDetectionBox->SetupAttachment(RootSceneComponent);
	TriggerDetectionBox->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f)); // 기본 크기
	TriggerDetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerDetectionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerDetectionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	TriggerDetectionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	CachedTurret = nullptr;
}

void ATriggeredTurret::BeginPlay()
{
	Super::BeginPlay();

	// Turret 액터 캐시
	if (TurretComponent)
	{
		CachedTurret = Cast<ATurret>(TurretComponent->GetChildActor());
	}

	// TriggerDetectionBox와 겹치는 모든 BaseTrigger 찾기
	if (TriggerDetectionBox)
	{
		// 오버랩 정보를 강제로 업데이트
		TriggerDetectionBox->UpdateOverlaps();

		// 방법 1: GetOverlappingActors 사용
		TArray<AActor*> OverlappingActors;
		TriggerDetectionBox->GetOverlappingActors(OverlappingActors, ABaseTrigger::StaticClass());

		for (AActor* Actor : OverlappingActors)
		{
			if (ABaseTrigger* Trigger = Cast<ABaseTrigger>(Actor))
			{
				ConnectedTriggers.AddUnique(Trigger);
			}
		}

		// 방법 2: 직접 오버랩 쿼리 사용 (GetOverlappingActors가 실패할 경우 대비)
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		bool bOverlap = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			TriggerDetectionBox->GetComponentLocation(),
			TriggerDetectionBox->GetComponentQuat(),
			ECC_WorldStatic,
			FCollisionShape::MakeBox(TriggerDetectionBox->GetScaledBoxExtent()),
			QueryParams
		);

		if (bOverlap)
		{
			for (const FOverlapResult& Result : OverlapResults)
			{
				if (ABaseTrigger* Trigger = Cast<ABaseTrigger>(Result.GetActor()))
				{
					ConnectedTriggers.AddUnique(Trigger);
				}
			}
		}

		// 델리게이트 바인딩
		for (ABaseTrigger* Trigger : ConnectedTriggers)
		{
			if (Trigger)
			{
				Trigger->OnTriggerStateChanged.AddDynamic(this, &ATriggeredTurret::OnTriggerStateChangedCallback);
				UE_LOG(LogTemp, Log, TEXT("[TriggeredTurret] Connected to trigger: %s"), *Trigger->GetName());
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[TriggeredTurret] Total found %d connected triggers"), ConnectedTriggers.Num());

		// 초기 트리거 상태 확인
		if (bAutoCheckTrigger)
		{
			CheckTriggerState();
		}
	}
}

void ATriggeredTurret::CheckTriggerState()
{
	if (!CachedTurret)
		return;

	// 모든 연결된 트리거의 상태를 확인하여 포탑 활성화 여부 결정
	bool bShouldActivate = CheckAllTriggersActive();

	// 포탑 상태 업데이트
	CachedTurret->SetActive(bShouldActivate);

	UE_LOG(LogTemp, Log, TEXT("[TriggeredTurret] CheckTriggerState: AllActive=%s, TurretActive=%s"),
		bShouldActivate ? TEXT("True") : TEXT("False"),
		CachedTurret->IsActive() ? TEXT("True") : TEXT("False"));
}

void ATriggeredTurret::OnTriggerStateChangedCallback(bool bNewState)
{
	if (!CachedTurret)
		return;

	// 트리거 상태 변경 시 모든 트리거 상태를 재확인
	bool bShouldActivate = CheckAllTriggersActive();
	CachedTurret->SetActive(bShouldActivate);

	UE_LOG(LogTemp, Log, TEXT("[TriggeredTurret] OnTriggerStateChanged: NewState=%s, AllActive=%s"),
		bNewState ? TEXT("True") : TEXT("False"),
		bShouldActivate ? TEXT("True") : TEXT("False"));
}

bool ATriggeredTurret::CheckAllTriggersActive() const
{
	// 연결된 트리거가 없으면 비활성화
	if (ConnectedTriggers.Num() == 0)
	{
		return false;
	}

	// 모든 트리거가 활성화되어 있는지 확인
	for (const ABaseTrigger* Trigger : ConnectedTriggers)
	{
		if (Trigger && !Trigger->IsTriggerActive())
		{
			// 하나라도 비활성화되어 있으면 false 반환
			return false;
		}
	}

	// 모든 트리거가 활성화되어 있으면 true 반환
	return true;
}

