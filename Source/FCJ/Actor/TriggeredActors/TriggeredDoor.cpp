// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TriggeredActors/TriggeredDoor.h"
#include "Actor/TriggeredActors/MovingDoor.h"
#include "Actor/Triggers/BaseTrigger.h"
#include "Components/ChildActorComponent.h"

ATriggeredDoor::ATriggeredDoor()
{
	// 델리게이트 기반 이벤트 처리를 사용하므로 Tick 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Door 자식 액터 컴포넌트 생성
	DoorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("DoorComponent"));
	DoorComponent->SetupAttachment(RootSceneComponent);
	// Door 클래스는 Blueprint에서 설정 (Child Actor Class)

	// Trigger1 자식 액터 컴포넌트 생성
	Trigger1Component = CreateDefaultSubobject<UChildActorComponent>(TEXT("Trigger1Component"));
	Trigger1Component->SetupAttachment(RootSceneComponent);
	// Trigger1 클래스는 Blueprint에서 설정 (Child Actor Class)

	// Trigger2 자식 액터 컴포넌트 생성
	Trigger2Component = CreateDefaultSubobject<UChildActorComponent>(TEXT("Trigger2Component"));
	Trigger2Component->SetupAttachment(RootSceneComponent);
	// Trigger2 클래스는 Blueprint에서 설정 (Child Actor Class)

	bTrigger1Active = false;
	bTrigger2Active = false;
	bDoorOpened = false;
	CachedDoor = nullptr;
	CachedTrigger1 = nullptr;
	CachedTrigger2 = nullptr;
}

void ATriggeredDoor::BeginPlay()
{
	Super::BeginPlay();

	// 자식 액터들을 캐시
	if (DoorComponent)
	{
		CachedDoor = Cast<AMovingDoor>(DoorComponent->GetChildActor());
	}

	if (Trigger1Component)
	{
		CachedTrigger1 = Cast<ABaseTrigger>(Trigger1Component->GetChildActor());
	}

	if (Trigger2Component)
	{
		CachedTrigger2 = Cast<ABaseTrigger>(Trigger2Component->GetChildActor());
	}

	// 트리거 델리게이트 바인딩
	if (CachedTrigger1)
	{
		CachedTrigger1->OnTriggerStateChanged.AddDynamic(this, &ATriggeredDoor::OnTrigger1StateChanged);
	}

	if (CachedTrigger2)
	{
		CachedTrigger2->OnTriggerStateChanged.AddDynamic(this, &ATriggeredDoor::OnTrigger2StateChanged);
	}
}

void ATriggeredDoor::OnTrigger1StateChanged(bool bNewState)
{
	// 이미 문이 열렸으면 더 이상 체크하지 않음
	if (bDoorOpened)
	{
		return;
	}

	bTrigger1Active = bNewState;
	CheckAndOpenDoor();
}

void ATriggeredDoor::OnTrigger2StateChanged(bool bNewState)
{
	// 이미 문이 열렸으면 더 이상 체크하지 않음
	if (bDoorOpened)
	{
		return;
	}

	bTrigger2Active = bNewState; 
	CheckAndOpenDoor();
}

void ATriggeredDoor::CheckAndOpenDoor()
{
	// 이미 문이 열렸으면 무시
	if (bDoorOpened)
	{
		return;
	}

	// 두 트리거가 모두 활성화되었는지 확인
	if (bTrigger1Active && bTrigger2Active)
	{
		if (CachedDoor)
		{
			// 문 열기 (서버에서만 실행됨)
			CachedDoor->OpenDoor();
			bDoorOpened = true;

			// 델리게이트 바인딩 해제 (더 이상 필요 없음)
			if (CachedTrigger1)
			{
				CachedTrigger1->OnTriggerStateChanged.RemoveDynamic(this, &ATriggeredDoor::OnTrigger1StateChanged);
			}

			if (CachedTrigger2)
			{
				CachedTrigger2->OnTriggerStateChanged.RemoveDynamic(this, &ATriggeredDoor::OnTrigger2StateChanged);
			}
		}
	}
}

