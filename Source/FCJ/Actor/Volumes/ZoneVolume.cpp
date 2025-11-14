// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Volumes/ZoneVolume.h"
#include "Actor/Triggers/ClearTrigger.h"
#include "Net/UnrealNetwork.h"

AZoneVolume::AZoneVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	// 레플리케이션 설정
	bReplicates = true;

	// 볼륨 박스 생성
	VolumeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("VolumeBox"));
	VolumeBox->SetBoxExtent(FVector(500.0f, 500.0f, 250.0f));
	VolumeBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VolumeBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	VolumeBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	VolumeBox->SetGenerateOverlapEvents(true);
	RootComponent = VolumeBox;

	bIsPuzzleZone = false;
	bIsZoneCleared = false;
	ZoneNumber = 0;
}

void AZoneVolume::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZoneVolume, bIsZoneCleared);
}

void AZoneVolume::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s BeginPlay - HasAuthority: %d, bIsPuzzleZone: %d"),
		*GetName(), HasAuthority(), bIsPuzzleZone);

	// 서버에서만 ClearTrigger 수집 및 바인딩
	if (HasAuthority() && bIsPuzzleZone)
	{
		CollectClearTriggers();
	}
}

void AZoneVolume::OnRep_IsZoneCleared()
{
	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s OnRep_IsZoneCleared - bIsZoneCleared: %d"),
		*GetName(), bIsZoneCleared);

	// 클라이언트에서 Zone 클리어 상태 변경 시 델리게이트 브로드캐스트
	if (bIsZoneCleared)
	{
		OnZoneCleared.Broadcast();
	}
}

void AZoneVolume::CollectClearTriggers()
{
	if (!VolumeBox)
	{
		UE_LOG(LogTemp, Error, TEXT("[ZoneVolume] %s - VolumeBox is null!"), *GetName());
		return;
	}

	// 오버랩 업데이트를 강제로 수행
	VolumeBox->UpdateOverlaps();

	// 디버깅: 모든 오버랩 액터 확인
	TArray<AActor*> AllOverlappingActors;
	VolumeBox->GetOverlappingActors(AllOverlappingActors);
	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - Total overlapping actors: %d"),
		*GetName(), AllOverlappingActors.Num());
	for (AActor* Actor : AllOverlappingActors)
	{
		if (Actor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - Overlapping actor: %s (Class: %s)"),
				*GetName(), *Actor->GetName(), *Actor->GetClass()->GetName());
		}
	}

	// 오버랩되는 ClearTrigger 액터만 가져오기
	TArray<AActor*> OverlappingActors;
	VolumeBox->GetOverlappingActors(OverlappingActors, AClearTrigger::StaticClass());

	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - Found %d overlapping actors (Class filter: AClearTrigger)"),
		*GetName(), OverlappingActors.Num());

	// ClearTrigger만 필터링하여 배열에 추가
	for (AActor* Actor : OverlappingActors)
	{
		AClearTrigger* ClearTrigger = Cast<AClearTrigger>(Actor);
		if (ClearTrigger && !ClearTriggers.Contains(ClearTrigger))
		{
			ClearTriggers.Add(ClearTrigger);
			UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - Added ClearTrigger: %s"),
				*GetName(), *ClearTrigger->GetName());

			// 델리게이트 바인딩
			ClearTrigger->OnClearStateChanged.AddDynamic(this, &AZoneVolume::OnClearTriggerStateChanged);
			UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - Delegate bound to ClearTrigger: %s"),
				*GetName(), *ClearTrigger->GetName());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - Total ClearTriggers collected: %d"),
		*GetName(), ClearTriggers.Num());

	// 초기 클리어 상태 확인
	CheckAllTriggersCleared();
}

void AZoneVolume::OnClearTriggerStateChanged(bool bIsCleared)
{
	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s OnClearTriggerStateChanged - bIsCleared: %d, HasAuthority: %d, bIsZoneCleared: %d"),
		*GetName(), bIsCleared, HasAuthority(), bIsZoneCleared);

	// 서버에서만 클리어 상태 체크
	if (!HasAuthority())
		return;

	// Zone이 이미 클리어되었다면 무시
	if (bIsZoneCleared)
		return;

	// 모든 트리거 클리어 여부 확인
	CheckAllTriggersCleared();
}

void AZoneVolume::CheckAllTriggersCleared()
{
	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s CheckAllTriggersCleared - bIsZoneCleared: %d, TriggerCount: %d"),
		*GetName(), bIsZoneCleared, ClearTriggers.Num());

	// Zone이 이미 클리어되었다면 무시
	if (bIsZoneCleared)
		return;

	// ClearTrigger가 없으면 체크하지 않음
	if (ClearTriggers.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - No ClearTriggers to check"), *GetName());
		return;
	}

	// 모든 ClearTrigger가 클리어되었는지 확인
	bool bAllCleared = true;
	int32 ClearedCount = 0;
	for (AClearTrigger* ClearTrigger : ClearTriggers)
	{
		if (!ClearTrigger || !ClearTrigger->IsCleared())
		{
			bAllCleared = false;
			if (ClearTrigger)
			{
				UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - ClearTrigger %s is NOT cleared yet"),
					*GetName(), *ClearTrigger->GetName());
			}
		}
		else
		{
			ClearedCount++;
			UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - ClearTrigger %s is cleared"),
				*GetName(), *ClearTrigger->GetName());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - Cleared: %d/%d, bAllCleared: %d"),
		*GetName(), ClearedCount, ClearTriggers.Num(), bAllCleared);

	// 모든 트리거가 클리어되었다면 Zone도 클리어
	if (bAllCleared)
	{
		bIsZoneCleared = true;
		UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - ZONE CLEARED! Broadcasting delegate"), *GetName());

		// 서버에서 델리게이트 브로드캐스트
		OnZoneCleared.Broadcast();

		// 더 이상 델리게이트를 들을 필요가 없으므로 모두 언바인딩
		for (AClearTrigger* ClearTrigger : ClearTriggers)
		{
			if (ClearTrigger)
			{
				ClearTrigger->OnClearStateChanged.RemoveDynamic(this, &AZoneVolume::OnClearTriggerStateChanged);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[ZoneVolume] %s - All delegates unbound"), *GetName());
	}
}

int32 AZoneVolume::GetClearedTriggerCount() const
{
	int32 Count = 0;
	for (AClearTrigger* ClearTrigger : ClearTriggers)
	{
		if (ClearTrigger && ClearTrigger->IsCleared())
		{
			Count++;
		}
	}
	return Count;
}
