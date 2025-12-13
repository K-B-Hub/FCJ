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

	UE_LOG(LogTemp, Warning, TEXT("[TriggeredDoor] %s BeginPlay - bUseORLogic: %d, bCanReopen: %d, OpenHeight: %.1f, OpenSpeed: %.1f"),
		*GetName(), bUseORLogic, bCanReopen, OpenHeight, OpenSpeed);

	// 자식 액터들을 캐시
	if (DoorComponent)
	{
		CachedDoor = Cast<AMovingDoor>(DoorComponent->GetChildActor());
		// 문 설정 적용
		if (CachedDoor)
		{
			CachedDoor->OpenHeight = OpenHeight;
			CachedDoor->OpenSpeed = OpenSpeed;
		}
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
	UE_LOG(LogTemp, Warning, TEXT("[TriggeredDoor] %s OnTrigger1StateChanged - NewState: %d"), *GetName(), bNewState);

	// bCanReopen이 false이고 이미 문이 열렸으면 더 이상 체크하지 않음
	if (!bCanReopen && bDoorOpened)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TriggeredDoor] %s - Door already opened and cannot reopen, ignoring"), *GetName());
		return;
	}

	bTrigger1Active = bNewState;
	CheckAndUpdateDoor();
}

void ATriggeredDoor::OnTrigger2StateChanged(bool bNewState)
{
	UE_LOG(LogTemp, Warning, TEXT("[TriggeredDoor] %s OnTrigger2StateChanged - NewState: %d"), *GetName(), bNewState);

	// bCanReopen이 false이고 이미 문이 열렸으면 더 이상 체크하지 않음
	if (!bCanReopen && bDoorOpened)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TriggeredDoor] %s - Door already opened and cannot reopen, ignoring"), *GetName());
		return;
	}

	bTrigger2Active = bNewState;
	CheckAndUpdateDoor();
}

void ATriggeredDoor::CheckAndUpdateDoor()
{
	if (!CachedDoor)
	{
		return;
	}

	// 트리거 조건 확인 (AND 또는 OR)
	bool bShouldOpen = false;
	if (bUseORLogic)
	{
		// OR 연산: 하나라도 활성화되면 문 열기
		bShouldOpen = bTrigger1Active || bTrigger2Active;
	}
	else
	{
		// AND 연산: 둘 다 활성화되면 문 열기
		bShouldOpen = bTrigger1Active && bTrigger2Active;
	}

	UE_LOG(LogTemp, Warning, TEXT("[TriggeredDoor] %s CheckAndUpdateDoor - bUseORLogic: %d, Trigger1: %d, Trigger2: %d, ShouldOpen: %d"),
		*GetName(), bUseORLogic, bTrigger1Active, bTrigger2Active, bShouldOpen);

	// 문 상태 업데이트
	if (bShouldOpen)
	{
		// 문 열기
		CachedDoor->OpenDoor();

		// 처음 열릴 때만 bDoorOpened를 true로 설정
		if (!bDoorOpened)
		{
			bDoorOpened = true;

			// bCanReopen이 false이면 델리게이트 바인딩 해제 (더 이상 상태 변경 필요 없음)
			if (!bCanReopen)
			{
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
	else
	{
		// bCanReopen이 true일 때만 문 닫기
		if (bCanReopen)
		{
			CachedDoor->CloseDoor();
		}
	}
}

